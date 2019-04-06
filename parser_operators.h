/*
 * Copyright (C) 2000, 2013 Thierry Crozat
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact: criezy01@gmail.com
 */

#ifndef parser_operators_h
#define parser_operators_h

#include <stdlib.h>
#include <math.h>
#include "str.h"
#include "list.h"
#include "strlist.h"
#include "math_utils.h"

#ifdef PARSER_TREE_DEBUG
#include <typeinfo>
#endif

/*! \class ParserOperator
 *
 * This is an internal class for the EquationParser. It is the base
 * class for all the operators (or functions) recognized by the parser.
 */
class ParserOperator {
public:
	virtual ~ParserOperator();

	virtual double evaluate() const = 0;

	// Use by operator =
	// Reimplement in classes that can change the argument value.
	virtual bool canBeModified() const;
	virtual double setValue(double value);

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return typeid(*this).name(); }
	virtual int nbChildren() const { return 0; }
	virtual ParserOperator* child(int) const { return NULL; }
#endif

protected:
	ParserOperator();
};

class ConstantOperator : public ParserOperator {
public:
	ConstantOperator(double c, const String& name = String());
	virtual ~ConstantOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const {
		if (name_.isEmpty())
			return String::format("Constant: %f", var_dbl);
		return String::format("Constant: %s (%f)", name_.c_str(), var_dbl);
	}
#endif

private:
	double var_dbl;
	String name_;
};

class VariableOperator : public ParserOperator {
public:
	VariableOperator(double *vardbl, const String& name);
	virtual ~VariableOperator();

	virtual double evaluate() const;

	virtual bool canBeModified() const;
	virtual double setValue(double value);
	const String& name() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const {
		return String::format("Variable: %s", name_.c_str());
	}
#endif

private:
	double *var_dbl;
	String name_;
};

class PrintOperator : public ParserOperator {
public:
	PrintOperator(const List<ParserOperator*>& values, const StringList& strings);
	virtual ~PrintOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Print"; }
	virtual int nbChildren() const {
		int cpt = 0;
		for (int i = 0 ; i < values_.size() ; ++i)
			if (values_[i] != NULL)
				++cpt;
		return cpt;
	}
	virtual ParserOperator* child(int idx) const {
		if (idx < 0)
			return NULL;
		int cpt = 0;
		for (int i = 0 ; i < values_.size() ; ++i) {
			if (values_[i] != NULL) {
				if (idx == cpt)
					return values_[i];
				else
					++cpt;
			}
		}
		return NULL;
	}
#endif

private:
	List<ParserOperator*> values_;
	StringList strings_;
};

class IfOperator : public ParserOperator {
public:
	IfOperator(ParserOperator *test, ParserOperator *left, ParserOperator *right);
	virtual ~IfOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "If"; }
	virtual int nbChildren() const { return 3; }
	virtual ParserOperator* child(int i) const { return i == 0 ? test : (i == 1 ? larg : (i == 2 ? rarg : NULL)); }
#endif

private:
	ParserOperator *test;
	ParserOperator *larg;
	ParserOperator *rarg;
};

class ParserOperator1 : public ParserOperator {
public:
	virtual ~ParserOperator1();

#ifdef PARSER_TREE_DEBUG
	virtual int nbChildren() const { return 1; }
	virtual ParserOperator* child(int i) const { return i == 0 ? arg : NULL; }
#endif

protected:
	ParserOperator1(ParserOperator *argument);

	ParserOperator *arg;
};

class ParserOperator2 : public ParserOperator {
public:
	virtual ~ParserOperator2();

#ifdef PARSER_TREE_DEBUG
	virtual int nbChildren() const { return 2; }
	virtual ParserOperator* child(int i) const { return i == 0 ? larg : (i == 1 ? rarg : NULL); }
#endif

protected:
	ParserOperator2(ParserOperator *left, ParserOperator *right);

	ParserOperator *larg;
	ParserOperator *rarg;
};

class OrOperator : public ParserOperator2 {
public:
	OrOperator(ParserOperator *left, ParserOperator *right);
	virtual ~OrOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Or"; }
#endif
};

class AndOperator : public ParserOperator2 {
public:
	AndOperator(ParserOperator *left, ParserOperator *right);
	virtual ~AndOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "And"; }
#endif
};

class EqualOperator : public ParserOperator2 {
public:
	EqualOperator(ParserOperator *left, ParserOperator *right);
	virtual ~EqualOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Is equal"; }
#endif
};

class GreaterOperator : public ParserOperator2 {
public:
	GreaterOperator(ParserOperator *left, ParserOperator *right);
	virtual ~GreaterOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Is greater"; }
#endif
};

class SmallerOperator : public ParserOperator2 {
public:
	SmallerOperator(ParserOperator *left, ParserOperator *right);
	virtual ~SmallerOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Is smaller"; }
#endif
};

class EqualOrGreaterOperator : public ParserOperator2 {
public:
	EqualOrGreaterOperator(ParserOperator *left, ParserOperator *right);
	virtual ~EqualOrGreaterOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Is equal or greater"; }
#endif
};

class EqualOrSmallerOperator : public ParserOperator2 {
public:
	EqualOrSmallerOperator(ParserOperator *left, ParserOperator *right);
	virtual ~EqualOrSmallerOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Is equal or smaller"; }
#endif
};

class NotEqualOperator : public ParserOperator2 {
public:
	NotEqualOperator(ParserOperator *left, ParserOperator *right);
	virtual ~NotEqualOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Is not equal"; }
#endif
};

class AssignmentOperator : public ParserOperator2 {
public:
	AssignmentOperator(ParserOperator *left, ParserOperator *right);
	virtual ~AssignmentOperator();

	virtual double evaluate() const;
	
	virtual bool canBeModified() const;
	virtual double setValue(double value);

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Assign"; }
#endif
};

class IncrementOperator : public ParserOperator2 {
public:
	IncrementOperator(ParserOperator *left, ParserOperator *right);
	virtual ~IncrementOperator();
 
	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Increment"; }
#endif
};

class SignOperator : public ParserOperator1 {
public:
	SignOperator(ParserOperator* argument);
	virtual ~SignOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Sign"; }
#endif
};

class NSignOperator : public ParserOperator1 {
public:
	NSignOperator(ParserOperator *argument);
	virtual ~NSignOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Change sign"; }
#endif
};

class PlusOperator : public ParserOperator2 {
public:
	PlusOperator(ParserOperator *left, ParserOperator *right);
	virtual ~PlusOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Add"; }
#endif
};

class MinusOperator : public ParserOperator2 {
public:
	MinusOperator(ParserOperator *left, ParserOperator *right);
	virtual ~MinusOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Substract"; }
#endif
};

class MultiplyOperator : public ParserOperator2 {
public:
	MultiplyOperator(ParserOperator *left, ParserOperator *right);
	virtual ~MultiplyOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Multiply"; }
#endif
};

class MultiplyAndAssignOperator : public ParserOperator2 {
public:
	MultiplyAndAssignOperator(ParserOperator *left, ParserOperator *right);
	virtual ~MultiplyAndAssignOperator();
 
	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Multiply and assign"; }
#endif
};

class DivideOperator : public ParserOperator2 {
public:
	DivideOperator(ParserOperator *left, ParserOperator *right);
	virtual ~DivideOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Divide"; }
#endif
};

class DivideAndAssignOperator : public ParserOperator2 {
public:
	DivideAndAssignOperator(ParserOperator *left, ParserOperator *right);
	virtual ~DivideAndAssignOperator();
 
	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Divide and assign"; }
#endif
};

class ModuloOperator : public ParserOperator2 {
public:
	ModuloOperator(ParserOperator *left, ParserOperator *right);
	virtual ~ModuloOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Modulo"; }
#endif
};


class SqrtOperator : public ParserOperator1 {
public:
	SqrtOperator(ParserOperator *argument);
	virtual ~SqrtOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Square root"; }
#endif
};

class CbrtOperator : public ParserOperator1 {
public:
	CbrtOperator(ParserOperator *argument);
	virtual ~CbrtOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Cubic root"; }
#endif
};

class CosOperator : public ParserOperator1 {
public:
	CosOperator(ParserOperator *argument);
	virtual ~CosOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Cosine"; }
#endif
};

class SinOperator : public ParserOperator1 {
public:
	SinOperator(ParserOperator *argument);
	virtual ~SinOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Sine"; }
#endif
};

class TanOperator : public ParserOperator1 {
public:
	TanOperator(ParserOperator *argument);
	virtual ~TanOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Tangent"; }
#endif
};

class ExpOperator : public ParserOperator1 {
public:
	ExpOperator(ParserOperator *argument);
	virtual ~ExpOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Exponential"; }
#endif
};

class LogOperator : public ParserOperator1 {
public:
	LogOperator(ParserOperator *argument);
	virtual ~LogOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Natural logarithm"; }
#endif
};

class Log10Operator : public ParserOperator1 {
public:
	Log10Operator(ParserOperator *argument);
	virtual ~Log10Operator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Base 10 logarithm"; }
#endif
};

class ASinOperator : public ParserOperator1 {
public:
	ASinOperator(ParserOperator *argument);
	virtual ~ASinOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Arc sine"; }
#endif
};

class ACosOperator : public ParserOperator1 {
public:
	ACosOperator(ParserOperator *argument);
	virtual ~ACosOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Arc cosine"; }
#endif
};


class ATanOperator : public ParserOperator1 {
public:
	ATanOperator(ParserOperator *argument);
	virtual ~ATanOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Arc tangent"; }
#endif
};

class ATan2Operator : public ParserOperator2 {
public:
	ATan2Operator(ParserOperator *left, ParserOperator *right);
	virtual ~ATan2Operator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Arc tangent of two arguments"; }
#endif
};

class SinHOperator : public ParserOperator1 {
public:
	SinHOperator(ParserOperator *argument);
	virtual ~SinHOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Hyperbolic sine"; }
#endif
};

class CosHOperator : public ParserOperator1 {
public:
	CosHOperator(ParserOperator *argument);
	virtual ~CosHOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Hyperbolic cosine"; }
#endif
};

class TanHOperator : public ParserOperator1 {
public:
	TanHOperator(ParserOperator *argument);
	virtual ~TanHOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Hyperbolic tangent"; }
#endif
};

class ASinHOperator : public ParserOperator1 {
public:
	ASinHOperator(ParserOperator *argument);
	virtual ~ASinHOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Inverse hyperbolic sine"; }
#endif
};

class ACosHOperator : public ParserOperator1 {
public:
	ACosHOperator(ParserOperator *argument);
	virtual ~ACosHOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Inverse hyperbolic cosine"; }
#endif
};

class ATanHOperator : public ParserOperator1 {
public:
	ATanHOperator(ParserOperator *argument);
	virtual ~ATanHOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Inverse hyperbolic tangent"; }
#endif
};

class RoundOperator : public ParserOperator1 {
public:
	RoundOperator(ParserOperator *argument);
	virtual ~RoundOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Round to nearest"; }
#endif
};

class CeilOperator : public ParserOperator1 {
public:
	CeilOperator(ParserOperator *argument);
	virtual ~CeilOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Round up"; }
#endif
};

class FloorOperator : public ParserOperator1 {
public:
	FloorOperator(ParserOperator *argument);
	virtual ~FloorOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Round down"; }
#endif
};

class FAbsOperator : public ParserOperator1 {
public:
	FAbsOperator(ParserOperator *argument);
	virtual ~FAbsOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Absolute value"; }
#endif
};

class PowOperator : public ParserOperator2 {
public:
	PowOperator(ParserOperator *left, ParserOperator *right);
	virtual ~PowOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Pow"; }
#endif
};

class Deg2RadOperator : public ParserOperator1 {
public:
	Deg2RadOperator(ParserOperator *argument);
	virtual ~Deg2RadOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Convert angle from degree to radian"; }
#endif
};

class Rad2DegOperator : public ParserOperator1 {
public:
	Rad2DegOperator(ParserOperator *argument);
	virtual ~Rad2DegOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Convert angle from radian to degree"; }
#endif
};

class MinimumOperator : public ParserOperator2 {
public:
	MinimumOperator(ParserOperator *left, ParserOperator *right);
	virtual ~MinimumOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Minimum"; }
#endif
};

class MaximumOperator : public ParserOperator2 {
public:
	MaximumOperator(ParserOperator *left, ParserOperator *right);
	virtual ~MaximumOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Maximum"; }
#endif
};

class URandOperator : public ParserOperator2 {
public:
	URandOperator(ParserOperator *min, ParserOperator *max);
	virtual ~URandOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Uniform distribution random number"; }
#endif
};

class NRandOperator : public ParserOperator2 {
public:
	NRandOperator(ParserOperator *mean, ParserOperator *sigma);
	virtual ~NRandOperator();

	virtual double evaluate() const;
	
	static double generateValue();

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Normal distribution random number"; }
#endif
};

class RandSeedOperator : public ParserOperator1 {
public:
	RandSeedOperator(ParserOperator *seed);
	virtual ~RandSeedOperator();

	virtual double evaluate() const;

#ifdef PARSER_TREE_DEBUG
	virtual String operatorName() const { return "Set seed for random numbers"; }
#endif
};

/***********************************************************
 * Inline Functions implementation
 ***********************************************************/

inline bool ParserOperator::canBeModified() const { return false;}
inline double ParserOperator::setValue(double value) {return value;}

inline double ConstantOperator::evaluate() const {return var_dbl;}

inline double VariableOperator::evaluate() const {return *var_dbl;}
inline bool VariableOperator::canBeModified() const { return true;}
inline double VariableOperator::setValue(double value) {return (*var_dbl = value);}
inline const String& VariableOperator::name() const {return name_;}

inline double IfOperator::evaluate() const {return (!MathUtils::isEqual(test->evaluate(), 0.) ? larg->evaluate() : rarg->evaluate());}

inline double OrOperator::evaluate() const {return (!MathUtils::isEqual(larg->evaluate(), 0.) || !MathUtils::isEqual(rarg->evaluate(), 0.) ? 1. : 0.);}

inline double AndOperator::evaluate() const {return (!MathUtils::isEqual(larg->evaluate(), 0.) && !MathUtils::isEqual(rarg->evaluate(), 0.) ? 1. : 0.);}

inline double EqualOperator::evaluate() const {return (MathUtils::isEqual(larg->evaluate(), rarg->evaluate()) ? 1. : 0.);}

inline double GreaterOperator::evaluate() const {return (larg->evaluate() > rarg->evaluate() ? 1. : 0.);}

inline double SmallerOperator::evaluate() const {return (larg->evaluate() < rarg->evaluate() ? 1. : 0.);}

inline double EqualOrGreaterOperator::evaluate() const {return (MathUtils::isSupOrEqual(larg->evaluate(), rarg->evaluate()) ? 1. : 0.);}

inline double EqualOrSmallerOperator::evaluate() const {return (MathUtils::isInfOrEqual(larg->evaluate(), rarg->evaluate()) ? 1. : 0.);}

inline double NotEqualOperator::evaluate() const {return (!MathUtils::isEqual(larg->evaluate(), rarg->evaluate()) ? 1. : 0.);}

inline double AssignmentOperator::evaluate() const {return larg->setValue(rarg->evaluate());}
// Assignment operator can be modified if right operand can be modified.
// This allows having a = b = c = 0; for example.
// Assigning to the right argument needs to also do the evaluation, and thus set the left argument afterward.
inline bool AssignmentOperator::canBeModified() const { return rarg->canBeModified();}
inline double AssignmentOperator::setValue(double value) {return larg->setValue(rarg->setValue(value));}

inline double IncrementOperator::evaluate() const {return larg->setValue(larg->evaluate() + rarg->evaluate());}

inline double SignOperator::evaluate() const {return arg->evaluate() < 0. ? -1. : 1.;}

inline double NSignOperator::evaluate() const {return -1.*arg->evaluate();}

inline double PlusOperator::evaluate() const {return larg->evaluate() + rarg->evaluate();}

inline double MinusOperator::evaluate() const {return larg->evaluate() - rarg->evaluate();}

inline double MultiplyOperator::evaluate() const {return larg->evaluate() * rarg->evaluate();}

inline double MultiplyAndAssignOperator::evaluate() const {return larg->setValue(larg->evaluate() * rarg->evaluate());}

inline double DivideOperator::evaluate() const {return larg->evaluate() / rarg->evaluate();}

inline double DivideAndAssignOperator::evaluate() const {return larg->setValue(larg->evaluate() / rarg->evaluate());}

inline double ModuloOperator::evaluate() const {return fmod(larg->evaluate(), rarg->evaluate());}

inline double SqrtOperator::evaluate() const {return sqrt(arg->evaluate());}

inline double CbrtOperator::evaluate() const {return cbrt(arg->evaluate());}

inline double CosOperator::evaluate() const {return cos(arg->evaluate());}

inline double SinOperator::evaluate() const {return sin(arg->evaluate());}

inline double TanOperator::evaluate() const {return tan(arg->evaluate());}

inline double ExpOperator::evaluate() const {return exp(arg->evaluate());}

inline double LogOperator::evaluate() const {return log(arg->evaluate());}

inline double Log10Operator::evaluate() const {return log10(arg->evaluate());}

inline double ASinOperator::evaluate() const {return asin(arg->evaluate());}

inline double ACosOperator::evaluate() const {return acos(arg->evaluate());}

inline double ATanOperator::evaluate() const {return atan(arg->evaluate());}

inline double ATan2Operator::evaluate() const {return atan2(larg->evaluate(), rarg->evaluate());}

inline double SinHOperator::evaluate() const {return sinh(arg->evaluate());}

inline double CosHOperator::evaluate() const {return cosh(arg->evaluate());}

inline double TanHOperator::evaluate() const {return tanh(arg->evaluate());}

inline double ASinHOperator::evaluate() const {return asinh(arg->evaluate());}

inline double ACosHOperator::evaluate() const {return acosh(arg->evaluate());}

inline double ATanHOperator::evaluate() const {return atanh(arg->evaluate());}

inline double RoundOperator::evaluate() const {
	double v = arg->evaluate();
	return (double)(int)(v < 0. ? (v - 0.5) : (v + 0.5));
}

inline double CeilOperator::evaluate() const {return ceil(arg->evaluate());}

inline double FloorOperator::evaluate() const {return floor(arg->evaluate());}

inline double FAbsOperator::evaluate() const {return fabs(arg->evaluate());}

inline double PowOperator::evaluate() const {return pow(larg->evaluate(),rarg->evaluate());}

inline double Deg2RadOperator::evaluate() const {return arg->evaluate() * M_PI / 180.;}

inline double Rad2DegOperator::evaluate() const {return arg->evaluate() * 180. / M_PI;}

inline double MinimumOperator::evaluate() const {
	double v1 = larg->evaluate(), v2 = rarg->evaluate();
	return v1 < v2 ? v1 : v2;
}

inline double MaximumOperator::evaluate() const {
	double v1 = larg->evaluate(), v2 = rarg->evaluate();
	return v1 < v2 ? v2 : v1;
}

inline double URandOperator::evaluate() const {
	double minimum = larg->evaluate(), maximum = rarg->evaluate();
	return minimum + rand() * (maximum - minimum) / RAND_MAX;
}

inline double NRandOperator::evaluate() const {
	return larg->evaluate() + rarg->evaluate() * generateValue();
}

inline double RandSeedOperator::evaluate() const {
	unsigned int s = (unsigned int)arg->evaluate();
	srand(s);
	return (double)s;
}



#endif
