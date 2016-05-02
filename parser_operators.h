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

protected:
	ParserOperator();
};

class ConstantOperator : public ParserOperator {
public:
	ConstantOperator(double c);
	virtual ~ConstantOperator();

	virtual double evaluate() const;

private:
	double var_dbl;
};

class VariableOperator : public ParserOperator {
public:
	VariableOperator(double *vardbl, const String& name);
	virtual ~VariableOperator();

	virtual double evaluate() const;

	virtual bool canBeModified() const;
	virtual double setValue(double value);
	const String& name() const;

private:
	double *var_dbl;
	String name_;
};

class PrintOperator : public ParserOperator {
public:
	PrintOperator(const List<ParserOperator*>& values, const StringList& strings);
	virtual ~PrintOperator();

	virtual double evaluate() const;

private:
	List<ParserOperator*> values_;
	StringList strings_;
};

class IfOperator : public ParserOperator {
public:
	IfOperator(ParserOperator *test, ParserOperator *left, ParserOperator *right);
	virtual ~IfOperator();

	virtual double evaluate() const;

private:
	ParserOperator *test;
	ParserOperator *larg;
	ParserOperator *rarg;
};


class OrOperator : public ParserOperator {
public:
	OrOperator(ParserOperator *left, ParserOperator *right);
	virtual ~OrOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class AndOperator : public ParserOperator {
public:
	AndOperator(ParserOperator *left, ParserOperator *right);
	virtual ~AndOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class EqualOperator : public ParserOperator {
public:
	EqualOperator(ParserOperator *left, ParserOperator *right);
	virtual ~EqualOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class GreaterOperator : public ParserOperator {
public:
	GreaterOperator(ParserOperator *left, ParserOperator *right);
	virtual ~GreaterOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class SmallerOperator : public ParserOperator {
public:
	SmallerOperator(ParserOperator *left, ParserOperator *right);
	virtual ~SmallerOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class EqualOrGreaterOperator : public ParserOperator {
public:
	EqualOrGreaterOperator(ParserOperator *left, ParserOperator *right);
	virtual ~EqualOrGreaterOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class EqualOrSmallerOperator : public ParserOperator {
public:
	EqualOrSmallerOperator(ParserOperator *left, ParserOperator *right);
	virtual ~EqualOrSmallerOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class NotEqualOperator : public ParserOperator {
public:
	NotEqualOperator(ParserOperator *left, ParserOperator *right);
	virtual ~NotEqualOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class AssignmentOperator : public ParserOperator {
public:
	AssignmentOperator(ParserOperator *left, ParserOperator *right);
	virtual ~AssignmentOperator();

	virtual double evaluate() const;
	
	virtual bool canBeModified() const;
	virtual double setValue(double value);

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class IncrementOperator : public ParserOperator {
public:
	IncrementOperator(ParserOperator *left, ParserOperator *right);
	virtual ~IncrementOperator();
 
	virtual double evaluate() const;
 
private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class NSignOperator : public ParserOperator {
public:
	NSignOperator(ParserOperator *argument);
	virtual ~NSignOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class PlusOperator : public ParserOperator {
public:
	PlusOperator(ParserOperator *left, ParserOperator *right);
	virtual ~PlusOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class MinusOperator : public ParserOperator {
public:
	MinusOperator(ParserOperator *left, ParserOperator *right);
	virtual ~MinusOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class MultiplyOperator : public ParserOperator {
public:
	MultiplyOperator(ParserOperator *left, ParserOperator *right);
	virtual ~MultiplyOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class MultiplyAndAssignOperator : public ParserOperator {
public:
	MultiplyAndAssignOperator(ParserOperator *left, ParserOperator *right);
	virtual ~MultiplyAndAssignOperator();
 
	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class DivideOperator : public ParserOperator {
public:
	DivideOperator(ParserOperator *left, ParserOperator *right);
	virtual ~DivideOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class DivideAndAssignOperator : public ParserOperator {
public:
	DivideAndAssignOperator(ParserOperator *left, ParserOperator *right);
	virtual ~DivideAndAssignOperator();
 
	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
};

class SqrtOperator : public ParserOperator {
public:
	SqrtOperator(ParserOperator *argument);
	virtual ~SqrtOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class CbrtOperator : public ParserOperator {
public:
	CbrtOperator(ParserOperator *argument);
	virtual ~CbrtOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class CosOperator : public ParserOperator {
public:
	CosOperator(ParserOperator *argument);
	virtual ~CosOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class SinOperator : public ParserOperator {
public:
	SinOperator(ParserOperator *argument);
	virtual ~SinOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class TanOperator : public ParserOperator {
public:
	TanOperator(ParserOperator *argument);
	virtual ~TanOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class ExpOperator : public ParserOperator {
public:
	ExpOperator(ParserOperator *argument);
	virtual ~ExpOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class LogOperator : public ParserOperator {
public:
	LogOperator(ParserOperator *argument);
	virtual ~LogOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class Log10Operator : public ParserOperator {
public:
	Log10Operator(ParserOperator *argument);
	virtual ~Log10Operator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class ASinOperator : public ParserOperator {
public:
	ASinOperator(ParserOperator *argument);
	virtual ~ASinOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class ACosOperator : public ParserOperator {
public:
	ACosOperator(ParserOperator *argument);
	virtual ~ACosOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};


class ATanOperator : public ParserOperator {
public:
	ATanOperator(ParserOperator *argument);
	virtual ~ATanOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class SinHOperator : public ParserOperator {
public:
	SinHOperator(ParserOperator *argument);
	virtual ~SinHOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class CosHOperator : public ParserOperator {
public:
	CosHOperator(ParserOperator *argument);
	virtual ~CosHOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class TanHOperator : public ParserOperator {
public:
	TanHOperator(ParserOperator *argument);
	virtual ~TanHOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class CeilOperator : public ParserOperator {
public:
	CeilOperator(ParserOperator *argument);
	virtual ~CeilOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class FloorOperator : public ParserOperator {
public:
	FloorOperator(ParserOperator *argument);
	virtual ~FloorOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class FAbsOperator : public ParserOperator {
public:
	FAbsOperator(ParserOperator *argument);
	virtual ~FAbsOperator();

	virtual double evaluate() const;

private:
	ParserOperator *arg;
};

class PowOperator : public ParserOperator {
public:
	PowOperator(ParserOperator *left, ParserOperator *right);
	virtual ~PowOperator();

	virtual double evaluate() const;

private:
	ParserOperator *larg;
	ParserOperator *rarg;
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

inline double NSignOperator::evaluate() const {return -1.*arg->evaluate();}

inline double PlusOperator::evaluate() const {return larg->evaluate() + rarg->evaluate();}

inline double MinusOperator::evaluate() const {return larg->evaluate() - rarg->evaluate();}

inline double MultiplyOperator::evaluate() const {return larg->evaluate() * rarg->evaluate();}

inline double MultiplyAndAssignOperator::evaluate() const {return larg->setValue(larg->evaluate() * rarg->evaluate());}

inline double DivideOperator::evaluate() const {return larg->evaluate() / rarg->evaluate();}

inline double DivideAndAssignOperator::evaluate() const {return larg->setValue(larg->evaluate() / rarg->evaluate());}

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

inline double SinHOperator::evaluate() const {return sinh(arg->evaluate());}

inline double CosHOperator::evaluate() const {return cosh(arg->evaluate());}

inline double TanHOperator::evaluate() const {return tanh(arg->evaluate());}

inline double CeilOperator::evaluate() const {return ceil(arg->evaluate());}

inline double FloorOperator::evaluate() const {return floor(arg->evaluate());}

inline double FAbsOperator::evaluate() const {return fabs(arg->evaluate());}

inline double PowOperator::evaluate() const {return pow(larg->evaluate(),rarg->evaluate());}


#endif
