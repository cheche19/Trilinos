/*@HEADER
// ***********************************************************************
// 
//       Ifpack: Object-Oriented Algebraic Preconditioner Package
//                 Copyright (2002) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ***********************************************************************
//@HEADER
*/

#ifndef IFPACK_RILUK_H
#define IFPACK_RILUK_H

#include "Ifpack_ConfigDefs.h"
#include "Ifpack_Preconditioner.h"
#include "Ifpack_Condest.h"
#include "Ifpack_ScalingType.h"
#include "Ifpack_IlukGraph.h"
#include "Epetra_CompObject.h"
#include "Epetra_MultiVector.h"
#include "Epetra_Vector.h"
#include "Epetra_CrsGraph.h"
#include "Epetra_CrsMatrix.h"
#include "Epetra_BlockMap.h"
#include "Epetra_Map.h"
#include "Epetra_Object.h"
#include "Epetra_Comm.h"
#include "Epetra_RowMatrix.h"

#ifdef HAVE_IFPACK_TEUCHOS
namespace Teuchos {
  class ParameterList;
}
#endif

//! Ifpack_Riluk: A class for constructing and using an incomplete lower/upper (ILU) factorization of a given Epetra_RowMatrix.

/*! The Ifpack_Riluk class computes a "Relaxed" ILU factorization with level k fill 
    of a given Epetra_CrsMatrix.  The factorization 
    that is produced is a function of several parameters:
<ol>
  <li> The pattern of the matrix - All fill is derived from the original matrix nonzero structure.  Level zero fill
       is defined as the original matrix pattern (nonzero structure), even if the matrix value at an entry is stored
       as a zero. (Thus it is possible to add entries to the ILU factors by adding zero entries the original matrix.)

  <li> Level of fill - Starting with the original matrix pattern as level fill of zero, the next level of fill is
       determined by analyzing the graph of the previous level and determining nonzero fill that is a result of combining
       entries that were from previous level only (not the current level).  This rule limits fill to entries that
       are direct decendents from the previous level graph.  Fill for level k is determined by applying this rule
       recursively.  For sufficiently large values of k, the fill would eventually be complete and an exact LU
       factorization would be computed.  Level of fill is defined during the construction of the Ifpack_IlukGraph object.

  <li> Level of overlap - All Ifpack preconditioners work on parallel distributed memory computers by using
       the row partitioning the user input matrix to determine the partitioning for local ILU factors.  If the level of
       overlap is set to zero,
       the rows of the user matrix that are stored on a given processor are treated as a self-contained local matrix
       and all column entries that reach to off-processor entries are ignored.  Setting the level of overlap to one
       tells Ifpack to increase the size of the local matrix by adding rows that are reached to by rows owned by this
       processor.  Increasing levels of overlap are defined recursively in the same way.  For sufficiently large levels
       of overlap, the entire matrix would be part of each processor's local ILU factorization process.
       Level of overlap is defined during the construction of the Ifpack_IlukGraph object.

       Once the factorization is computed, applying the factorization \(LUy = x\) 
       results in redundant approximations for any elements of y that correspond to 
       rows that are part of more than one local ILU factor.  The OverlapMode (changed by calling SetOverlapMode())
       defines how these redundancies are
       handled using the Epetra_CombineMode enum.  The default is to zero out all values of y for rows that
       were not part of the original matrix row distribution.

  <li> Fraction of relaxation - Ifpack_Riluk computes the ILU factorization row-by-row.  As entries at a given
       row are computed, some number of them will be dropped because they do match the prescribed sparsity pattern.
       The relaxation factor determines how these dropped values will be handled.  If the RelaxValue (changed by calling
       SetRelaxValue()) is zero, then these extra entries will by dropped.  This is a classical ILU approach.
       If the RelaxValue is 1, then the sum
       of the extra entries will be added to the diagonal.  This is a classical Modified ILU (MILU) approach.  If
       RelaxValue is between 0 and 1, then RelaxValue times the sum of extra entries will be added to the diagonal.

       For most situations, RelaxValue should be set to zero.  For certain kinds of problems, e.g., reservoir modeling,
       there is a conservation principle involved such that any operator should obey a zero row-sum property.  MILU 
       was designed for these cases and you should set the RelaxValue to 1.  For other situations, setting RelaxValue to
       some nonzero value may improve the stability of factorization, and can be used if the computed ILU factors
       are poorly conditioned.

  <li> Diagonal perturbation - Prior to computing the factorization, it is possible to modify the diagonal entries of the matrix
       for which the factorization will be computing.  If the absolute and relative perturbation values are zero and one,
       respectively, the
       factorization will be compute for the original user matrix A.  Otherwise, the factorization
       will computed for a matrix that differs from the original user matrix in the diagonal values only.  Below we discuss
       the details of diagonal perturbations.
       The absolute and relative threshold values are set by calling SetAbsoluteThreshold() and SetRelativeThreshold(), respectively.
</ol>

<b>\e A \e priori Diagonal Perturbations</b>

Given the above method to estimate the conditioning of the incomplete factors,
if we detect that our factorization is too ill-conditioned
we can improve the conditioning by perturbing the matrix diagonal and
restarting the factorization using
this more diagonally dominant matrix.  In order to apply perturbation,
prior to starting
the factorization, we compute a diagonal perturbation of our matrix
\f$A\f$ and perform the factorization on this perturbed
matrix.  The overhead cost of perturbing the diagonal is minimal since
the first step in computing the incomplete factors is to copy the
matrix \f$A\f$ into the memory space for the incomplete factors.  We
simply compute the perturbed diagonal at this point. 

The actual perturbation values we use are the diagonal values \f$(d_1, d_2, \ldots, d_n)\f$
with \f$d_i = sgn(d_i)\alpha + d_i\rho\f$, \f$i=1, 2, \ldots, n\f$, where
\f$n\f$ is the matrix dimension and \f$sgn(d_i)\f$ returns
the sign of the diagonal entry.  This has the effect of
forcing the diagonal values to have minimal magnitude of \f$\alpha\f$ and
to increase each by an amount proportional to \f$\rho\f$, and still keep
the sign of the original diagonal entry.

<b>Constructing Ifpack_Riluk objects</b>

Constructing Ifpack_Riluk objects is a multi-step process.  The basic steps are as follows:
<ol>
  <li> Create Ifpack_Riluk instance, including storage,  via constructor.
  <li> Enter values via one or more Put or SumInto functions.
  <li> Complete construction via FillComplete call.
</ol>

Note that, even after a matrix is constructed, it is possible to update existing matrix entries.  It is \e not possible to
create new entries.

<b> Counting Floating Point Operations </b>

Each Ifpack_Riluk object keep track of the number
of \e serial floating point operations performed using the specified object as the \e this argument
to the function.  The Flops() function returns this number as a double precision number.  Using this 
information, in conjunction with the Epetra_Time class, one can get accurate parallel performance
numbers.  The ResetFlops() function resets the floating point counter.

\warning A Epetra_Map is required for the Ifpack_Riluk constructor.

*/    

class Ifpack_Riluk: public Ifpack_Preconditioner {
      
 public:
  Ifpack_Riluk(Epetra_RowMatrix* A);
  
  //! Ifpack_Riluk Destructor
  ~Ifpack_Riluk();

  int Initialize();
  
  bool IsInitialized() const
  {
    return(IsInitialized_);
  }

  //! Initialize L and U with values from user matrix A.
  /*! Copies values from the user's matrix into the nonzero pattern of L and U.
    \param In 
           A - User matrix to be factored.
    \warning The graph of A must be identical to the graph passed in to Ifpack_IlukGraph constructor.
             
   */
  int ComputeSetup();

#ifdef HAVE_IFPACK_TEUCHOS
  //! Set parameters using a Teuchos::ParameterList object.
  /* This method is only available if the Teuchos package is enabled.
     This method recognizes four parameter names: relax_value,
     absolute_threshold, relative_threshold and overlap_mode. These names are
     case insensitive, and in each case except overlap_mode, the ParameterEntry
     must have type double. For overlap_mode, the ParameterEntry must have
     type Epetra_CombineMode.
  */
  int SetParameters(Teuchos::ParameterList& parameterlist);
#endif
  int SetParameter(const string Name, const int Value)
  {
    return(-1);
  }

  int SetParameter(const string Name, const double Value)
  {
    return(-1);
  }

  //! Compute ILU factors L and U using the specified graph, diagonal perturbation thresholds and relaxation parameters.
  /*! This function computes the RILU(k) factors L and U using the current:
    <ol>
    <li> Ifpack_IlukGraph specifying the structure of L and U.
    <li> Value for the RILU(k) relaxation parameter.
    <li> Value for the \e a \e priori diagonal threshold values.
    </ol>
    InitValues() must be called before the factorization can proceed.
   */
  int Compute();

  //! If factor is completed, this query returns true, otherwise it returns false.
  bool IsComputed() const 
  {
    return(IsComputed_);
  }

  // Mathematical functions.
  
  int Apply(const Epetra_MultiVector& X, 
	       Epetra_MultiVector& Y) const
  {
    return(Multiply(false,X,Y));
  }
  int Multiply(bool Trans, const Epetra_MultiVector& X, 
	       Epetra_MultiVector& Y) const;

  //! Returns the result of a Ifpack_Riluk forward/back solve on a Epetra_MultiVector X in Y (works for Epetra_Vectors also).
  /*! 
    \param In
    Trans -If true, solve transpose problem.
    \param In
    X - A Epetra_MultiVector of dimension NumVectors to solve for.
    \param Out
    Y -A Epetra_MultiVector of dimension NumVectorscontaining result.
    
    \return Integer error code, set to 0 if successful.
  */
  int Solve(bool Trans, const Epetra_MultiVector& X, Epetra_MultiVector& Y) const;

  double Condest(const Ifpack_CondestType CT = Ifpack_Cheap, 
		 Epetra_RowMatrix* Matrix = 0);

  // Atribute access functions
  
  //! Get RILU(k) relaxation parameter
  double GetRelaxValue() {return RelaxValue_;}

  //! Get absolute threshold value
  double GetAbsoluteThreshold() {return Athresh_;}

  //! Get relative threshold value
  double GetRelativeThreshold() {return Rthresh_;}

  //! Returns the number of global matrix rows.
  int NumGlobalRows() const {return(Graph().NumGlobalRows());};
  
  //! Returns the number of global matrix columns.
  int NumGlobalCols() const {return(Graph().NumGlobalCols());};
  
  //! Returns the number of nonzero entries in the global graph.
  int NumGlobalNonzeros() const {return(L().NumGlobalNonzeros()+U().NumGlobalNonzeros());};
  
  //! Returns the number of diagonal entries found in the global input graph.
  virtual int NumGlobalBlockDiagonals() const {return(Graph().NumGlobalBlockDiagonals());};
  
  //! Returns the number of local matrix rows.
  int NumMyRows() const {return(Graph().NumMyRows());};
  
  //! Returns the number of local matrix columns.
  int NumMyCols() const {return(Graph().NumMyCols());};
  
  //! Returns the number of nonzero entries in the local graph.
  int NumMyNonzeros() const {return(L().NumMyNonzeros()+U().NumMyNonzeros());};
  
  //! Returns the number of diagonal entries found in the local input graph.
  virtual int NumMyBlockDiagonals() const {return(Graph().NumMyBlockDiagonals());};
  
  //! Returns the number of nonzero diagonal values found in matrix.
  virtual int NumMyDiagonals() const {return(NumMyDiagonals_);};
  
  //! Returns the index base for row and column indices for this graph.
  int IndexBase() const {return(Graph().IndexBase());};
  
  //! Returns the address of the Ifpack_IlukGraph associated with this factored matrix.
  const Ifpack_IlukGraph & Graph() const {return(*Graph_);};
  
  //! Returns the address of the L factor associated with this factored matrix.
  const Epetra_CrsMatrix & L() const {return(*L_);};
    
  //! Returns the address of the D factor associated with this factored matrix.
  const Epetra_Vector & D() const {return(*D_);};
    
  //! Returns the address of the L factor associated with this factored matrix.
  const Epetra_CrsMatrix & U() const {return(*U_);};

  //! Returns a character string describing the operator
  char * Label() const {return((char*)Label_);}
      
  int SetLabel(const char* Label)
  {
    strcpy(Label_,Label);
    return(0);
  }
  
    //! If set true, transpose of this operator will be applied.
    /*! This flag allows the transpose of the given operator to be used implicitly.  Setting this flag
        affects only the Apply() and ApplyInverse() methods.  If the implementation of this interface 
	does not support transpose use, this method should return a value of -1.
      
    \param In
	   UseTranspose -If true, multiply by the transpose of operator, otherwise just use operator.

    \return Always returns 0.
  */
  int SetUseTranspose(bool UseTranspose) {UseTranspose_ = UseTranspose; return(0);};

    //! Returns the result of a Epetra_Operator inverse applied to an Epetra_MultiVector X in Y.
    /*! In this implementation, we use several existing attributes to determine how virtual
        method ApplyInverse() should call the concrete method Solve().  We pass in the UpperTriangular(), 
	the Epetra_CrsMatrix::UseTranspose(), and NoDiagonal() methods. The most notable warning is that
	if a matrix has no diagonal values we assume that there is an implicit unit diagonal that should
	be accounted for when doing a triangular solve.

    \param In
	   X - A Epetra_MultiVector of dimension NumVectors to solve for.
    \param Out
	   Y -A Epetra_MultiVector of dimension NumVectors containing result.

    \return Integer error code, set to 0 if successful.
  */
  int ApplyInverse(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const 
  {
    return(Solve(Ifpack_Riluk::UseTranspose(), X, Y));
  };

    //! Returns 0.0 because this class cannot compute Inf-norm.
    double NormInf() const {return(0.0);};

    //! Returns false because this class cannot compute an Inf-norm.
    bool HasNormInf() const {return(false);};

    //! Returns the current UseTranspose setting.
    bool UseTranspose() const {return(UseTranspose_);};

    //! Returns the Epetra_Map object associated with the domain of this operator.
    const Epetra_Map & OperatorDomainMap() const {return(U_->OperatorDomainMap());};

    //! Returns the Epetra_Map object associated with the range of this operator.
    const Epetra_Map & OperatorRangeMap() const{return(L_->OperatorRangeMap());};

    //! Returns the Epetra_BlockMap object associated with the range of this matrix operator.
    const Epetra_Comm & Comm() const{return(Comm_);};
  //@}

const Epetra_RowMatrix& Matrix() const
{
  return(*A_);
}

Epetra_RowMatrix& Matrix()
{
  return(*A_);
}

virtual ostream& Print(ostream& os) const
{
  return(os);
}

private:

  Epetra_RowMatrix* A_;
  
  int InitAllValues(const Epetra_RowMatrix & A, int MaxNumEntries);
  Ifpack_IlukGraph* Graph_;
  Epetra_CrsGraph* CrsGraph_;
  Epetra_Map * IlukRowMap_;
  Epetra_Map * IlukDomainMap_;
  Epetra_Map * IlukRangeMap_;
  const Epetra_Map * U_DomainMap_;
  const Epetra_Map * L_RangeMap_;
  const Epetra_Comm & Comm_;
  Epetra_CrsMatrix * L_;
  Epetra_CrsMatrix * U_;
  Epetra_CrsGraph * L_Graph_;
  Epetra_CrsGraph * U_Graph_;
  Epetra_Vector * D_;
  bool UseTranspose_;

  int NumMyDiagonals_;
  bool Allocated_;
  bool ValuesInitialized_;
  bool Factored_;
  double RelaxValue_;
  double Athresh_;
  double Rthresh_;
  double Condest_;
  int CondestMaxIters_;
  double CondestTol_;
  int LevelOfFill_;

  bool IsInitialized_;
  bool IsComputed_;
  char Label_[160];

};

#endif /* IFPACK_RILUK_H */
