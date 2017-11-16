// @HEADER
// ************************************************************************
//
//                           Intrepid2 Package
//                 Copyright (2007) Sandia Corporation
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
// Questions? Contact Kyungjoo Kim  (kyukim@sandia.gov), or
//                    Mauro Perego  (mperego@sandia.gov)
//
// ************************************************************************
// @HEADER

/** \file   Intrepid2_HVOL_TET_Cn_FEMDef.hpp
    \brief  Definition file for FEM basis functions of degree n for H(vol) functions on TET
    \author Created by M. Perego, based on the Intrepid2::HGRAD_TET_Cn_FEM class
*/

#ifndef __INTREPID2_HVOL_TET_CN_FEM_DEF_HPP__
#define __INTREPID2_HVOL_TET_CN_FEM_DEF_HPP__

#include "Intrepid2_HGRAD_TET_Cn_FEM_ORTH.hpp"

namespace Intrepid2 {

  // -------------------------------------------------------------------------------------
  
  namespace Impl {

    template<EOperator opType>
    template<typename outputViewType,
             typename inputViewType,
             typename workViewType,
             typename vinvViewType>
    KOKKOS_INLINE_FUNCTION
    void
    Basis_HVOL_TET_Cn_FEM::Serial<opType>::
    getValues(       outputViewType output,
               const inputViewType  input,
                     workViewType   work,
               const vinvViewType   vinv ) {

      constexpr ordinal_type spaceDim = 3;
      const ordinal_type 
        card = vinv.dimension(0),
        npts = input.dimension(0);

      // compute order
      ordinal_type order = 0;
      for (ordinal_type p=0;p<=Parameters::MaxOrder;++p) {
        if (card == Intrepid2::getPnCardinality<spaceDim>(p)) {
          order = p;
          break;
        }
      }
      
      typedef typename Kokkos::DynRankView<typename workViewType::value_type, typename workViewType::memory_space> viewType;
      auto vcprop = Kokkos::common_view_alloc_prop(work);
      auto ptr = work.data();

      switch (opType) {
      case OPERATOR_VALUE: {
        const viewType phis(Kokkos::view_wrap(ptr, vcprop), card, npts);
        workViewType dummyView;
        
        Impl::Basis_HGRAD_TET_Cn_FEM_ORTH::
          Serial<opType>::getValues(phis, input, dummyView, order);
        
        for (ordinal_type i=0;i<card;++i)
          for (ordinal_type j=0;j<npts;++j) {
            output(i,j) = 0.0;
            for (ordinal_type k=0;k<card;++k)
              output(i,j) += vinv(k,i)*phis(k,j);
          }
        break;
      }
      case OPERATOR_GRAD:
      case OPERATOR_D1: {
        const viewType phis(Kokkos::view_wrap(ptr, vcprop), card, npts, spaceDim);
        ptr += card*npts*spaceDim*get_dimension_scalar(work);
        const viewType workView(Kokkos::view_wrap(ptr, vcprop), card, npts, spaceDim+1);
        Impl::Basis_HGRAD_TET_Cn_FEM_ORTH::
          Serial<opType>::getValues(phis, input, workView, order);

        for (ordinal_type i=0;i<card;++i)
          for (ordinal_type j=0;j<npts;++j)
            for (ordinal_type k=0;k<spaceDim;++k) {
              output(i,j,k) = 0.0;
              for (ordinal_type l=0;l<card;++l)
                output(i,j,k) += vinv(l,i)*phis(l,j,k);
            }
        break;
      }
      case OPERATOR_D2:
      case OPERATOR_D3:
      case OPERATOR_D4:
      case OPERATOR_D5:
      case OPERATOR_D6:
      case OPERATOR_D7:
      case OPERATOR_D8:
      case OPERATOR_D9:
      case OPERATOR_D10: {
        const ordinal_type dkcard = getDkCardinality<opType,spaceDim>(); //(orDn + 1);
        const viewType phis(Kokkos::view_wrap(ptr, vcprop), card, npts, dkcard);
        workViewType dummyView;

        Impl::Basis_HGRAD_TET_Cn_FEM_ORTH::
          Serial<opType>::getValues(phis, input, dummyView, order);

        for (ordinal_type i=0;i<card;++i)
          for (ordinal_type j=0;j<npts;++j)
            for (ordinal_type k=0;k<dkcard;++k) {
              output(i,j,k) = 0.0;
              for (ordinal_type l=0;l<card;++l)
                output(i,j,k) += vinv(l,i)*phis(l,j,k);
            }
        break;
      }
      default: {
        INTREPID2_TEST_FOR_ABORT( true,
                                  ">>> ERROR (Basis_HVOL_TET_Cn_FEM): Operator type not implemented");
      }
      }
    }

    template<typename SpT, ordinal_type numPtsPerEval,
             typename outputValueValueType, class ...outputValueProperties,
             typename inputPointValueType,  class ...inputPointProperties,
             typename vinvValueType,        class ...vinvProperties>
    void
    Basis_HVOL_TET_Cn_FEM::
    getValues(       Kokkos::DynRankView<outputValueValueType,outputValueProperties...> outputValues,
               const Kokkos::DynRankView<inputPointValueType, inputPointProperties...>  inputPoints,
               const Kokkos::DynRankView<vinvValueType,       vinvProperties...>        vinv,
               const EOperator operatorType) {
      typedef          Kokkos::DynRankView<outputValueValueType,outputValueProperties...>         outputValueViewType;
      typedef          Kokkos::DynRankView<inputPointValueType, inputPointProperties...>          inputPointViewType;
      typedef          Kokkos::DynRankView<vinvValueType,       vinvProperties...>                vinvViewType;
      typedef typename ExecSpace<typename inputPointViewType::execution_space,SpT>::ExecSpaceType ExecSpaceType;
      
      // loopSize corresponds to cardinality
      const auto loopSizeTmp1 = (inputPoints.dimension(0)/numPtsPerEval);
      const auto loopSizeTmp2 = (inputPoints.dimension(0)%numPtsPerEval != 0);
      const auto loopSize = loopSizeTmp1 + loopSizeTmp2;
      Kokkos::RangePolicy<ExecSpaceType,Kokkos::Schedule<Kokkos::Static> > policy(0, loopSize);

      typedef typename inputPointViewType::value_type inputPointType;

      const ordinal_type cardinality = outputValues.dimension(0);
      const ordinal_type spaceDim = 3;

      ordinal_type order = 0;
      while((Intrepid2::getPnCardinality<spaceDim>(++order) !=  cardinality) && (order != Parameters::MaxOrder));

      auto vcprop = Kokkos::common_view_alloc_prop(inputPoints);
      typedef typename Kokkos::DynRankView< inputPointType, typename inputPointViewType::memory_space> workViewType;

      switch (operatorType) {
      case OPERATOR_VALUE: {
        auto bufferSize = Basis_HVOL_TET_Cn_FEM::Serial<OPERATOR_VALUE>::getWorkSizePerPoint(order);
        workViewType  work(Kokkos::view_alloc("Basis_HVOL_TET_Cn_FEM::getValues::work", vcprop), bufferSize, inputPoints.dimension(0));
        typedef Functor<outputValueViewType,inputPointViewType,vinvViewType, workViewType,
            OPERATOR_VALUE,numPtsPerEval> FunctorType;
        Kokkos::parallel_for( policy, FunctorType(outputValues, inputPoints, vinv, work) );
      break;
      }
      case OPERATOR_GRAD:
      case OPERATOR_D1: { 
        auto bufferSize = Basis_HVOL_TET_Cn_FEM::Serial<OPERATOR_D1>::getWorkSizePerPoint(order);
        workViewType  work(Kokkos::view_alloc("Basis_HVOL_TET_Cn_FEM::getValues::work", vcprop), bufferSize, inputPoints.dimension(0));
        typedef Functor<outputValueViewType,inputPointViewType,vinvViewType, workViewType,
            OPERATOR_D1,numPtsPerEval> FunctorType;
        Kokkos::parallel_for( policy, FunctorType(outputValues, inputPoints, vinv, work) );
        break;
      }
      case OPERATOR_D2: {
        auto bufferSize = Basis_HVOL_TET_Cn_FEM::Serial<OPERATOR_D2>::getWorkSizePerPoint(order);
        typedef Functor<outputValueViewType,inputPointViewType,vinvViewType, workViewType,
            OPERATOR_D2,numPtsPerEval> FunctorType;
        workViewType  work(Kokkos::view_alloc("Basis_HVOL_TET_Cn_FEM::getValues::work", vcprop), bufferSize, inputPoints.dimension(0));
        Kokkos::parallel_for( policy, FunctorType(outputValues, inputPoints, vinv, work) );
        break;
      }
    /*  case OPERATOR_D3: {
        typedef Functor<outputValueViewType,inputPointViewType,vinvViewType, workViewType
            OPERATOR_D3,numPtsPerEval> FunctorType;
        workViewType  work(Kokkos::view_alloc("Basis_HVOL_TET_Cn_FEM::getValues::work", vcprop), cardinality, inputPoints.dimension(0), outputValues.dimension(2));
        Kokkos::parallel_for( policy, FunctorType(outputValues, inputPoints, vinv, work) );
        break;
      }*/
      default: {
        INTREPID2_TEST_FOR_EXCEPTION( true , std::invalid_argument,
                                      ">>> ERROR (Basis_HVOL_TET_Cn_FEM): Operator type not implemented" );
        break;
      }
      }
    }
  }
  
  // -------------------------------------------------------------------------------------
  template<typename SpT, typename OT, typename PT>
  Basis_HVOL_TET_Cn_FEM<SpT,OT,PT>::
  Basis_HVOL_TET_Cn_FEM( const ordinal_type order,
                          const EPointType   pointType ) {
    constexpr ordinal_type spaceDim = 3;

    this->basisCardinality_  = Intrepid2::getPnCardinality<spaceDim>(order); // bigN
    this->basisDegree_       = order; // small n
    this->basisCellTopology_ = shards::CellTopology(shards::getCellTopologyData<shards::Tetrahedron<4> >() );
    this->basisType_         = BASIS_FEM_FIAT;
    this->basisCoordinates_  = COORDINATES_CARTESIAN;

    const ordinal_type card = this->basisCardinality_;

    // points are computed in the host and will be copied
    Kokkos::DynRankView<scalarType,typename SpT::array_layout,Kokkos::HostSpace>
      dofCoords("HVOL::Tet::Cn::dofCoords", card, spaceDim);
    
    // construct lattice (only internal nodes for HVOL element)
    const ordinal_type offset = 1;
    PointTools::getLattice( dofCoords,
                            this->basisCellTopology_,
                            order+spaceDim+offset, offset,
                            pointType );
    
    this->dofCoords_ = Kokkos::create_mirror_view(typename SpT::memory_space(), dofCoords);
    Kokkos::deep_copy(this->dofCoords_, dofCoords);

    // form Vandermonde matrix.  Actually, this is the transpose of the VDM,
    // so we transpose on copy below.
    const ordinal_type lwork = card*card;
    Kokkos::DynRankView<scalarType,Kokkos::LayoutLeft,Kokkos::HostSpace>
      vmat("HVOL::Tet::Cn::vmat", card, card),
      work("HVOL::Tet::Cn::work", lwork),
      ipiv("HVOL::Tet::Cn::ipiv", card);
  
    Impl::Basis_HGRAD_TET_Cn_FEM_ORTH::getValues<Kokkos::HostSpace::execution_space,Parameters::MaxNumPtsPerBasisEval>(vmat, dofCoords, order, OPERATOR_VALUE);

    ordinal_type info = 0;
    Teuchos::LAPACK<ordinal_type,scalarType> lapack;
    
    lapack.GETRF(card, card,
                 vmat.data(), vmat.stride_1(),
                 (ordinal_type*)ipiv.data(),
                 &info);

    INTREPID2_TEST_FOR_EXCEPTION( info != 0,
                                  std::runtime_error ,
                                  ">>> ERROR: (Intrepid2::Basis_HVOL_TET_Cn_FEM) lapack.GETRF returns nonzero info." );

    lapack.GETRI(card,
                 vmat.data(), vmat.stride_1(),
                 (ordinal_type*)ipiv.data(),
                 work.data(), lwork,
                 &info);

    INTREPID2_TEST_FOR_EXCEPTION( info != 0,
                                  std::runtime_error ,
                                  ">>> ERROR: (Intrepid2::Basis_HVOL_TET_Cn_FEM) lapack.GETRI returns nonzero info." );

    // create host mirror
    Kokkos::DynRankView<scalarType,typename SpT::array_layout,Kokkos::HostSpace>
      vinv("HVOL::Line::Cn::vinv", card, card);

    for (ordinal_type i=0;i<card;++i)
      for (ordinal_type j=0;j<card;++j)
        vinv(i,j) = vmat(j,i);

    this->vinv_ = Kokkos::create_mirror_view(typename SpT::memory_space(), vinv);
    Kokkos::deep_copy(this->vinv_ , vinv);

    // initialize tags
    {
      // Basis-dependent initializations
      constexpr ordinal_type tagSize  = 4;        // size of DoF tag, i.e., number of fields in the tag
      const ordinal_type posScDim = 0;        // position in the tag, counting from 0, of the subcell dim 
      const ordinal_type posScOrd = 1;        // position in the tag, counting from 0, of the subcell ordinal
      const ordinal_type posDfOrd = 2;        // position in the tag, counting from 0, of DoF ordinal relative to the subcell
      
      constexpr ordinal_type maxCard = Intrepid2::getPnCardinality<spaceDim, Parameters::MaxOrder>();
      ordinal_type tags[maxCard][tagSize];

      const ordinal_type 
        numElemDof = this->basisCardinality_; //all the degrees of freedom are internal.


      ordinal_type elemId = 0;
      for (ordinal_type i=0;i<this->basisCardinality_;++i) {
        // elem
        tags[i][0] = spaceDim; // intr dof
        tags[i][1] = 0; // intr id
        tags[i][2] = elemId++; // local dof id
        tags[i][3] = numElemDof; // total vert dof
      }

      ordinal_type_array_1d_host tagView(&tags[0][0], card*tagSize);
      
      // Basis-independent function sets tag and enum data in tagToOrdinal_ and ordinalToTag_ arrays:
      // tags are constructed on host
      this->setOrdinalTagData(this->tagToOrdinal_,
                              this->ordinalToTag_,
                              tagView,
                              this->basisCardinality_,
                              tagSize,
                              posScDim,
                              posScOrd,
                              posDfOrd);
    }
  }
} // namespace Intrepid2
#endif
