// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER


#ifndef ROL_INTERIORPOINT_PRIMALDUAL_RESIDUAL_H
#define ROL_INTERIORPOINT_PRIMALDUAL_RESIDUAL_H

#include "ROL_Elementwise_Function.hpp"
#include "ROL_EqualityConstraint.hpp"
#include "ROL_LinearOperator.hpp"
#include "ROL_Objective.hpp"
#include "ROL_PartitionedVector.hpp"

namespace ROL {
namespace InteriorPoint {

/** @ingroup func_group
    \class   ROL::InteriorPoint::PrimalDualResidual
    \brief   Express the Primal-Dual Interior Point gradient
             as an equality constraint 
    
             See Nocedal & Wright second edition equation (19.6)
             In that book the convention for naming components
      
             x - optimization variable (here subscript o)
             s - slack variable (here subscript s)
             y - Lagrange multiplier for the equality constraint (here subscript e)
             z - Lagrange multiplier for the inequality constraint (here subscript i)
    --- 
*/


template<class Real> class PrimalDualSymmetrizer;


template<class Real>
class PrimalDualResidual : public EqualityConstraint<Real> {

private:
  typedef Vector<Real>             V;
  typedef PartitionedVector<Real>  PV;
  typedef Objective<Real>          OBJ;
  typedef EqualityConstraint<Real> CON;


  typedef typename PV::size_type   size_type;

  Teuchos::RCP<OBJ> obj_;    // Objective function
  Teuchos::RCP<CON> eqcon_;  // Equality Constraint
  Teuchos::RCP<CON> incon_;  // Inequality Constraint

  Teuchos::RCP<V> qo_;       // Storage for optimization variables
  Teuchos::RCP<V> qs_;       // Storage for slack variables
  Teuchos::RCP<V> qe_;       // Storage for equality multiplier variables
  Teuchos::RCP<V> qi_;       // Storage for inequality multiplier variables

  Real mu_;                  // Penalty parameter

  Teuchos::RCP<LinearOperator<Real> > sym_;

  const static size_type OPT   = 0;  // Optimization vector
  const static size_type SLACK = 1;  // Slack vector
  const static size_type EQUAL = 2;  // Lagrange multipliers for equality constraint
  const static size_type INEQ  = 3;  // Lagrange multipliers for inequality constraint



public:

  PrimalDualResidual( const Teuchos::RCP<OBJ> &obj, 
                      const Teuchos::RCP<CON> &eqcon,
                      const Teuchos::RCP<CON> &incon,
                      const V& x ) : 
                      obj_(obj), eqcon_(eqcon), incon_(incon), mu_(1.0) {

    // Allocate storage vectors
    const PV &xpv = Teuchos::dyn_cast<const PV>(x);

    qo_ = xpv.get(OPT)->clone();
    qs_ = xpv.get(SLACK)->clone();
    qe_ = xpv.get(EQUAL)->clone();
    qi_ = xpv.get(INEQ)->clone();

    sym_ = Teuchos::rcp( new PrimalDualSymmetrizer<Real>(*qs_) );

  }

  void value( V &c, const V &x, Real &tol ) {

    using Teuchos::RCP;

    // Downcast to partitioned vectors
    PV &cpv = Teuchos::dyn_cast<PV>(c);
    const PV &xpv = Teuchos::dyn_cast<const PV>(x);

    RCP<const V> xo = xpv.get(OPT);
    RCP<const V> xs = xpv.get(SLACK);
    RCP<const V> xe = xpv.get(EQUAL);
    RCP<const V> xi = xpv.get(INEQ);

    c.zero();    

    RCP<V> co = cpv.get(OPT);
    RCP<V> cs = cpv.get(SLACK);
    RCP<V> ce = cpv.get(EQUAL);
    RCP<V> ci = cpv.get(INEQ);

    // Optimization components
    obj_->gradient(*co,*xo,tol); 

    // Apply adjoint equality Jacobian at xo to xe and store in qo
    eqcon_->applyAdjointJacobian(*qo_,*xe,*xo,tol);
    co->axpy(-1.0,*qo_);

    incon_->applyAdjointJacobian(*qo_,*xi,*xo,tol);
    co->axpy(-1.0,*qo_);

    // Slack components
    cs->set(*xs);

    Elementwise::Multiply<Real> mult;
    cs->applyBinary(mult,*xi);
 
    Elementwise::Fill<Real> fill(-mu_);
    qs_->applyUnary(fill);

    cs->plus(*qs_);               // Sz-e

    // Equality component
    eqcon_->value(*ce, *xo, tol); // c_e(x)

    // Inequality component
    incon_->value(*ci, *xo, tol); // c_i(x)-s
    ci->axpy(-1.0, *xs); 

    sym_->update(*xs);
    sym_->apply(c,c,tol);
    sym_->applyInverse(c,c,tol);

  }   

  void applyJacobian( V &jv, const V &v, const V &x, Real &tol ) {

    using Teuchos::RCP;

    jv.zero();

    // Downcast to partitioned vectors
    PV &jvpv = Teuchos::dyn_cast<PV>(jv);
    const PV &vpv = Teuchos::dyn_cast<const PV>(v);
    const PV &xpv = Teuchos::dyn_cast<const PV>(x);

    RCP<V> jvo = jvpv.get(OPT);
    RCP<V> jvs = jvpv.get(SLACK);
    RCP<V> jve = jvpv.get(EQUAL);
    RCP<V> jvi = jvpv.get(INEQ);

    RCP<const V> vo = vpv.get(OPT);
    RCP<const V> vs = vpv.get(SLACK);
    RCP<const V> ve = vpv.get(EQUAL);
    RCP<const V> vi = vpv.get(INEQ);

    RCP<const V> xo = xpv.get(OPT);
    RCP<const V> xs = xpv.get(SLACK);
    RCP<const V> xe = xpv.get(EQUAL);
    RCP<const V> xi = xpv.get(INEQ);

    // Optimization components
    obj_->hessVec(*jvo,*vo,*xo,tol);
    
    eqcon_->applyAdjointHessian(*qo_,*xe,*vo,*xo,tol);

    jvo->axpy(-1.0,*qo_);

    incon_->applyAdjointHessian(*qo_,*xi,*vo,*xo,tol);
    
    jvo->axpy(-1.0,*qo_);
    
    eqcon_->applyAdjointJacobian(*qo_,*ve,*xo,tol);

    jvo->axpy(-1.0,*qo_);
 
    incon_->applyAdjointJacobian(*qo_,*vi,*xo,tol);

    jvo->axpy(-1.0,*qo_);
    

    // Slack components
    jvs->set(*vs);

    Elementwise::Multiply<Real> mult;
 
    jvs->applyBinary(mult,*xi);

    qs_->set(*vi);

    qs_->applyBinary(mult,*xs);
    
    jvs->plus(*qs_);

    // Equality component
    eqcon_->applyJacobian(*jve,*vo,*xo,tol);
    
    // Inequality components
    incon_->applyJacobian(*jvi,*vo,*xo,tol);
    
    jvi->axpy(-1.0,*vs);

    sym_->update(*xs);
    sym_->apply(jv,jv,tol);
    sym_->applyInverse(jv,jv,tol); 

  }

  void updatePenalty( Real mu ) { 
    mu_ = mu;
  }

};  // class PrimalDualResidual



// Applying this operator to the left- and right-hand-sides, will
// symmetrize the Primal-Dual Interior-Point KKT system, yielding
// equation (19.13) from Nocedal & Wright

template<class Real> 
class PrimalDualSymmetrizer : public LinearOperator<Real> {

  typedef Vector<Real>             V;  
  typedef PartitionedVector<Real>  PV;

  typedef typename PV::size_type   size_type;

private:
  Teuchos::RCP<V> s_;

  const static size_type OPT   = 0;  // Optimization vector
  const static size_type SLACK = 1;  // Slack vector
  const static size_type EQUAL = 2;  // Lagrange multipliers for equality constraint
  const static size_type INEQ  = 3;  // Lagrange multipliers for inequality constraint

public:
  
  PrimalDualSymmetrizer( const V &s ) {
    s_ = s.clone();
    s_->set(s);
  }

  void update( const V& s, bool flag = true, int iter = -1 ) {
    s_->set(s);  
  }

  void apply( V &Hv, const V &v, Real &tol ) const {

    using Teuchos::RCP;
    using Teuchos::dyn_cast;

    const PV &vpv = dyn_cast<const PV>(v);
    PV &Hvpv = dyn_cast<PV>(Hv);

    RCP<const V> vo = vpv.get(OPT);
    RCP<const V> vs = vpv.get(SLACK);
    RCP<const V> ve = vpv.get(EQUAL);
    RCP<const V> vi = vpv.get(INEQ);

    RCP<V> Hvo = Hvpv.get(OPT);
    RCP<V> Hvs = Hvpv.get(SLACK);
    RCP<V> Hve = Hvpv.get(EQUAL);
    RCP<V> Hvi = Hvpv.get(INEQ);

    Hvo->set(*vo);

    Hvs->set(*vs);
    Elementwise::Divide<Real> div;
    Hvs->applyBinary(div,*s_);

    Hve->set(*ve);
    Hve->scale(-1.0);

    Hvi->set(*vi);
    Hvi->scale(-1.0);

  }

  void applyInverse( V &Hv, const V &v, Real &tol ) const {

    using Teuchos::RCP;
    using Teuchos::dyn_cast;

    const PV &vpv = dyn_cast<const PV>(v);
    PV &Hvpv = dyn_cast<PV>(Hv);

    RCP<const V> vo = vpv.get(OPT);
    RCP<const V> vs = vpv.get(SLACK);
    RCP<const V> ve = vpv.get(EQUAL);
    RCP<const V> vi = vpv.get(INEQ);

    RCP<V> Hvo = Hvpv.get(OPT);
    RCP<V> Hvs = Hvpv.get(SLACK);
    RCP<V> Hve = Hvpv.get(EQUAL);
    RCP<V> Hvi = Hvpv.get(INEQ);

    Hvo->set(*vo);

    Hvs->set(*vs);
    Elementwise::Multiply<Real> mult;
    Hvs->applyBinary(mult,*s_);

    Hve->set(*ve);
    Hve->scale(-1.0);

    Hvi->set(*vi);
    Hvi->scale(-1.0);

  }
}; // class PrimalDualSymmetrizer


} // namespace InteriorPoint





} // namespace ROL


#endif // ROL_INTERIORPOINT_PRIMALDUAL_RESIDUAL_H

