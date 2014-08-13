#ifndef FSTDISTANCELP_H
#define FSTDISTANCELP_H

/*!======================================================================
   Feature Selection Toolbox 3 source code
   ---------------------------------------
	
   \file    distance_Lp.hpp
   \brief   Implements L_{p} distance, p equals numerator/denominator (template parameters)
   \author  Petr Somol (somol@utia.cas.cz) with collaborators, see Contacts at http://fst.utia.cz
   \date    March 2011
   \version 3.1.0.beta
   \note    FST3 was developed using gcc 4.3 and requires
   \note    \li Boost library (http://www.boost.org/, tested with versions 1.33.1 and 1.44),
   \note    \li (\e optionally) LibSVM (http://www.csie.ntu.edu.tw/~cjlin/libsvm/, 
                tested with version 3.00)
   \note    Note that LibSVM is required for SVM related tools only,
            as demonstrated in demo12t.cpp, demo23.cpp, demo25t.cpp, demo32t.cpp, etc.

*/ /* 
=========================================================================
Copyright:
  * FST3 software (with exception of any externally linked libraries) 
    is copyrighted by Institute of Information Theory and Automation (UTIA), 
    Academy of Sciences of the Czech Republic.
  * FST3 source codes as presented here do not contain code of third parties. 
    FST3 may need linkage to external libraries to exploit its functionality
    in full. For details on obtaining and possible usage restrictions 
    of external libraries follow their original sources (referenced from
    FST3 documentation wherever applicable).
  * FST3 software is available free of charge for non-commercial use. 
    Please address all inquires concerning possible commercial use 
    of FST3, or if in doubt, to FST3 maintainer (see http://fst.utia.cz).
  * Derivative works based on FST3 are permitted as long as they remain
    non-commercial only.
  * Re-distribution of FST3 software is not allowed without explicit
    consent of the copyright holder.
Disclaimer of Warranty:
  * FST3 software is presented "as is", without warranty of any kind, 
    either expressed or implied, including, but not limited to, the implied 
    warranties of merchantability and fitness for a particular purpose. 
    The entire risk as to the quality and performance of the program 
    is with you. Should the program prove defective, you assume the cost 
    of all necessary servicing, repair or correction.
Limitation of Liability:
  * The copyright holder will in no event be liable to you for damages, 
    including any general, special, incidental or consequential damages 
    arising out of the use or inability to use the code (including but not 
    limited to loss of data or data being rendered inaccurate or losses 
    sustained by you or third parties or a failure of the program to operate 
    with any other programs).
========================================================================== */

#include <boost/smart_ptr.hpp>
#include <iostream>
#include <cmath>
#include "error.hpp"
#include "global.hpp"
#include "distance.hpp"

/*============== Template parameter type naming conventions ==============
--------- Numeric types: -------------------------------------------------
DATATYPE - data sample values - usually real numbers (but may be integers
          in text processing etc.)
REALTYPE - must be real numbers - for representing intermediate results of 
          calculations like mean, covariance etc.
IDXTYPE - index values for enumeration of data samples - (nonnegative) integers, 
          extent depends on numbers of samples in data
DIMTYPE - index values for enumeration of features (dimensions), or classes (not 
          class sizes) - (nonnegative) integers, usually lower extent than IDXTYPE, 
          but be aware of expressions like _classes*_features*_features ! 
          in linearized representations of feature matrices for all classes
BINTYPE - feature selection marker type - represents ca. <10 different feature 
          states (selected, deselected, sel./desel. temporarily 1st nested loop, 2nd...)
RETURNTYPE - criterion value: real value, but may be extended in future to support 
          multiple values 
--------- Class types: ---------------------------------------------------
SUBSET       - class of class type Subset 
CLASSIFIER   - class implementing interface defined in abstract class Classifier 
EVALUATOR    - class implementing interface defined in abstract class Sequential_Step 
DISTANCE     - class implementing interface defined in abstract class Distance 
DATAACCESSOR - class implementing interface defined in abstract class Data_Accessor 
INTERVALCONTAINER - class of class type TIntervaller 
CONTAINER    - STL container of class type TInterval  
========================================================================== */

namespace FST {

/*! \brief Implements L_{p} distance, p equals numerator/denominator (template parameters) */
template<typename DATATYPE, typename REALTYPE, typename DIMTYPE, class SUBSET, const int numerator=1, const int denominator=1>
class Distance_Lp : public Distance<DATATYPE,DIMTYPE,SUBSET> {
public:
	typedef Distance<DATATYPE,DIMTYPE,SUBSET> parent;
	Distance_Lp() : _p(REALTYPE(numerator)/REALTYPE(denominator)), parent::_n(0), parent::_d(0) {notify("Distance_Lp (empty) constructor.");}
	Distance_Lp(const DIMTYPE n) : _p(REALTYPE(numerator)/REALTYPE(denominator)) {parent::reset(n); notify("Distance_Lp constructor.");}
	Distance_Lp(const Distance_Lp& dlp) : Distance<DATATYPE,DIMTYPE,SUBSET>(dlp), _p(dlp._p) {notify("Distance_Lp copy-constructor.");}
	virtual ~Distance_Lp() {notify("Distance_Lp destructor.");}
	
	virtual DATATYPE distance(const DATATYPE *pattern1, const DATATYPE *pattern2);
	
	Distance_Lp* clone() const {return new Distance_Lp(*this);}
	
	virtual std::ostream& print(std::ostream& os) const {os << "Distance_Lp(p=" << _p << ")"; return os;}
protected:
	const REALTYPE _p;
};

template<typename DATATYPE, typename REALTYPE, typename DIMTYPE, class SUBSET, const int numerator, const int denominator>
DATATYPE Distance_Lp<DATATYPE,REALTYPE,DIMTYPE,SUBSET,numerator,denominator>::distance(const DATATYPE *pattern1, const DATATYPE *pattern2)
{
	assert(parent::_index);
	assert(parent::_d>0);
	assert(pattern1!=NULL);
	assert(pattern2!=NULL);
	DATATYPE result=0;
	DIMTYPE index_i;
	for(DIMTYPE i=0;i<parent::_d;i++) {
		index_i=parent::_index[i];
		result+=pow(abs(pattern1[index_i]-pattern2[index_i]),_p);
	}
	return pow(result,1.0/_p);
}

} // namespace
#endif // FSTDISTANCELP_H ///:~
