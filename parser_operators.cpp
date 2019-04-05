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

#include "parser_operators.h"
#include "redirect_output.h"
#include <stdio.h>
#include <string.h>

ParserOperator::ParserOperator() {}
ParserOperator::~ParserOperator() {}


ConstantOperator::ConstantOperator(double c, const String& name) : ParserOperator(), var_dbl(c), name_(name) {}
ConstantOperator::~ConstantOperator() {}

VariableOperator::VariableOperator(double *vardbl, const String& name) : ParserOperator(), var_dbl(vardbl), name_(name) {}
VariableOperator::~VariableOperator() {}

PrintOperator::PrintOperator(const List<ParserOperator*>& values, const StringList& strings) :
	ParserOperator(), values_(values), strings_(strings) {}
PrintOperator::~PrintOperator() {
	for (int i = 0 ; i < values_.size() ; ++i)
		delete values_[i];
}

double PrintOperator::evaluate() const {
	if (values_.size() == 1) {
		const VariableOperator* var = dynamic_cast<const VariableOperator*>(values_.first());
		if (var != NULL && !var->name().isEmpty()) {
			double value = var->evaluate();
			// special case when print contains just a variable
			rprintf("%s = %.12g\n", var->name().c_str(), value);
			return value;
		}
	}

	double value = 0.0;
	int str_i = 0;
	for (int i = 0 ; i < values_.size() ; ++i) {
		if (values_[i] == NULL) {
			if (str_i < strings_.size())
				rprintf("%s", strings_[str_i++].c_str());
		} else {
			value = values_[i]->evaluate();
			rprintf("%.12g", value);
		}
		if (i < values_.size() - 1)
			rprintf(" ");
		else
			rprintf("\n");
	}
	return value;
}

ParserOperator1::ParserOperator1(ParserOperator *argument) : ParserOperator(), arg(argument) {}

ParserOperator1::~ParserOperator1() {
	delete arg;
}

ParserOperator2::ParserOperator2(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {}

ParserOperator2::~ParserOperator2() {
	delete larg;
	delete rarg;
}

SignOperator::SignOperator(ParserOperator *argument) : ParserOperator1(argument) {}

SignOperator::~SignOperator() {}

NSignOperator::NSignOperator(ParserOperator *argument) : ParserOperator1(argument) {}
NSignOperator::~NSignOperator() {}

IfOperator::IfOperator(ParserOperator *test_exp, ParserOperator *left, ParserOperator *right) :
	ParserOperator(), test(test_exp), larg(left), rarg(right) {}

IfOperator::~IfOperator() {
	delete test;
	delete larg;
	delete rarg;
}

OrOperator::OrOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
OrOperator::~OrOperator() {}

AndOperator::AndOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
AndOperator::~AndOperator() {}

EqualOperator::EqualOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
EqualOperator::~EqualOperator() {}

GreaterOperator::GreaterOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
GreaterOperator::~GreaterOperator() {}

SmallerOperator::SmallerOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
SmallerOperator::~SmallerOperator() {}

EqualOrGreaterOperator::EqualOrGreaterOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
EqualOrGreaterOperator::~EqualOrGreaterOperator() {}

EqualOrSmallerOperator::EqualOrSmallerOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
EqualOrSmallerOperator::~EqualOrSmallerOperator() {}

NotEqualOperator::NotEqualOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
NotEqualOperator::~NotEqualOperator() {}

AssignmentOperator::AssignmentOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
AssignmentOperator::~AssignmentOperator() {}

IncrementOperator::IncrementOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
IncrementOperator::~IncrementOperator() {}

PlusOperator::PlusOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
PlusOperator::~PlusOperator() {}

MinusOperator::MinusOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
MinusOperator::~MinusOperator() {}

MultiplyOperator::MultiplyOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
MultiplyOperator::~MultiplyOperator() {}

MultiplyAndAssignOperator::MultiplyAndAssignOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
MultiplyAndAssignOperator::~MultiplyAndAssignOperator() {}

DivideOperator::DivideOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
DivideOperator::~DivideOperator() {}

DivideAndAssignOperator::DivideAndAssignOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
DivideAndAssignOperator::~DivideAndAssignOperator() {}

ModuloOperator::ModuloOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
ModuloOperator::~ModuloOperator() {}

SqrtOperator::SqrtOperator(ParserOperator *argument) : ParserOperator1(argument){}
SqrtOperator::~SqrtOperator() {}

CbrtOperator::CbrtOperator(ParserOperator *argument) : ParserOperator1(argument) {}
CbrtOperator::~CbrtOperator() {}

CosOperator::CosOperator(ParserOperator *argument) : ParserOperator1(argument) {}
CosOperator::~CosOperator() {}

SinOperator::SinOperator(ParserOperator *argument) : ParserOperator1(argument) {}
SinOperator::~SinOperator() {}

TanOperator::TanOperator(ParserOperator *argument) : ParserOperator1(argument) {}
TanOperator::~TanOperator() {}

ExpOperator::ExpOperator(ParserOperator *argument) : ParserOperator1(argument) {}
ExpOperator::~ExpOperator() {}

LogOperator::LogOperator(ParserOperator *argument) : ParserOperator1(argument) {}
LogOperator::~LogOperator() {}

Log10Operator::Log10Operator(ParserOperator *argument) : ParserOperator1(argument) {}
Log10Operator::~Log10Operator() {}

ASinOperator::ASinOperator(ParserOperator *argument) : ParserOperator1(argument) {}
ASinOperator::~ASinOperator() {}

ACosOperator::ACosOperator(ParserOperator *argument) : ParserOperator1(argument) {}
ACosOperator::~ACosOperator() {}

ATanOperator::ATanOperator(ParserOperator *argument) : ParserOperator1(argument) {}
ATanOperator::~ATanOperator() {}

ATan2Operator::ATan2Operator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
ATan2Operator::~ATan2Operator() {}

SinHOperator::SinHOperator(ParserOperator *argument) : ParserOperator1(argument) {}
SinHOperator::~SinHOperator() {}

CosHOperator::CosHOperator(ParserOperator *argument) : ParserOperator1(argument) {}
CosHOperator::~CosHOperator() {}

TanHOperator::TanHOperator(ParserOperator *argument) : ParserOperator1(argument) {}
TanHOperator::~TanHOperator() {}

ASinHOperator::ASinHOperator(ParserOperator *argument) : ParserOperator1(argument) {}
ASinHOperator::~ASinHOperator() {}

ACosHOperator::ACosHOperator(ParserOperator *argument) : ParserOperator1(argument) {}
ACosHOperator::~ACosHOperator() {}

ATanHOperator::ATanHOperator(ParserOperator *argument) : ParserOperator1(argument) {}
ATanHOperator::~ATanHOperator() {}

RoundOperator::RoundOperator(ParserOperator *argument) : ParserOperator1(argument) {}
RoundOperator::~RoundOperator() {}

CeilOperator::CeilOperator(ParserOperator *argument) : ParserOperator1(argument) {}
CeilOperator::~CeilOperator() {}

FloorOperator::FloorOperator(ParserOperator *argument) : ParserOperator1(argument) {}
FloorOperator::~FloorOperator() {}

FAbsOperator::FAbsOperator(ParserOperator *argument) : ParserOperator1(argument) {}
FAbsOperator::~FAbsOperator() {}


PowOperator::PowOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
PowOperator::~PowOperator() {}

Deg2RadOperator::Deg2RadOperator(ParserOperator *argument) : ParserOperator1(argument) {}
Deg2RadOperator::~Deg2RadOperator() {}

Rad2DegOperator::Rad2DegOperator(ParserOperator *argument) : ParserOperator1(argument) {}
Rad2DegOperator::~Rad2DegOperator() {}

MinimumOperator::MinimumOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
MinimumOperator::~MinimumOperator() {}

MaximumOperator::MaximumOperator(ParserOperator *left, ParserOperator *right) : ParserOperator2(left, right) {}
MaximumOperator::~MaximumOperator() {}

URandOperator::URandOperator(ParserOperator *minimum, ParserOperator *maximum) : ParserOperator2(minimum, maximum) {}
URandOperator::~URandOperator() {}

NRandOperator::NRandOperator(ParserOperator *mean, ParserOperator *sigma) : ParserOperator2(mean, sigma) {}
NRandOperator::~NRandOperator() {}

double NRandOperator::generateValue() {
	// Generate a random number using a normal distribution with mean = 0 and sigma = 1.
	// Method described by Abramowitz and Stegun
	static double U, V;
	static int phase = 0;
	double Z;
	if (phase == 0) {
		U = (rand() + 1.) / (RAND_MAX + 2.);
		V = rand() / (RAND_MAX + 1.);
		Z = sqrt(-2 * log(U)) * sin(2. * M_PI * V);
	} else
		Z = sqrt(-2 * log(U)) * cos(2. * M_PI * V);
	phase = 1 - phase;
	return Z;
}

RandSeedOperator::RandSeedOperator(ParserOperator *seed) : ParserOperator1(seed) {}
RandSeedOperator::~RandSeedOperator() {}

