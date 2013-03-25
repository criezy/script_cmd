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
#include <stdio.h>
#include <string.h>

ParserOperator::ParserOperator() {}
ParserOperator::~ParserOperator() {}


ConstantOperator::ConstantOperator(double c) : ParserOperator(), var_flt((float)c), var_dbl(c) {
}
ConstantOperator::~ConstantOperator() {}

VariableOperator::VariableOperator(double *vardbl, const String& name) : ParserOperator(), var_dbl(vardbl), name_(name) {
}
VariableOperator::~VariableOperator() {
}

PrintOperator::PrintOperator(ParserOperator *variable) : ParserOperator(), variable_(variable) {
}
PrintOperator::~PrintOperator() {
	delete variable_;
}

double PrintOperator::evaluate() const {
	double value = variable_->evaluate();
	const VariableOperator* var = dynamic_cast<const VariableOperator*>(variable_);
	if (var != NULL && !var->name().isEmpty())
		printf("%s = %.12g\n", var->name().c_str(), value);
	else
		printf("%.12g\n", value);
	return value;
}

NSignOperator::NSignOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
NSignOperator::~NSignOperator() {
	delete arg;
}

IfOperator::IfOperator(ParserOperator *test_exp, ParserOperator *left, ParserOperator *right) :
	ParserOperator(), test(test_exp), larg(left), rarg(right)
{
}

IfOperator::~IfOperator() {
	delete test;
	delete larg;
	delete rarg;
}

OrOperator::OrOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
OrOperator::~OrOperator() {
	delete larg;
	delete rarg;
}

AndOperator::AndOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
AndOperator::~AndOperator() {
	delete larg;
	delete rarg;
}

EqualOperator::EqualOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
EqualOperator::~EqualOperator() {
	delete larg;
	delete rarg;
}

GreaterOperator::GreaterOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
GreaterOperator::~GreaterOperator() {
	delete larg;
	delete rarg;
}

SmallerOperator::SmallerOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
SmallerOperator::~SmallerOperator() {
	delete larg;
	delete rarg;
}

EqualOrGreaterOperator::EqualOrGreaterOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
EqualOrGreaterOperator::~EqualOrGreaterOperator() {
	delete larg;
	delete rarg;
}

EqualOrSmallerOperator::EqualOrSmallerOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
EqualOrSmallerOperator::~EqualOrSmallerOperator() {
	delete larg;
	delete rarg;
}

NotEqualOperator::NotEqualOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
NotEqualOperator::~NotEqualOperator() {
	delete larg;
	delete rarg;
}

AssignmentOperator::AssignmentOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
AssignmentOperator::~AssignmentOperator() {
	delete larg;
	delete rarg;
}

PlusOperator::PlusOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
PlusOperator::~PlusOperator() {
	delete larg;
	delete rarg;
}

MinusOperator::MinusOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
MinusOperator::~MinusOperator() {
	delete larg;
	delete rarg;
}

MultiplyOperator::MultiplyOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
MultiplyOperator::~MultiplyOperator() {
	delete larg;
	delete rarg;
}

DivideOperator::DivideOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
DivideOperator::~DivideOperator() {
	delete larg;
	delete rarg;
}

SqrtOperator::SqrtOperator(ParserOperator *argument) : ParserOperator(), arg(argument){
}
SqrtOperator::~SqrtOperator() {
	delete arg;
}

CbrtOperator::CbrtOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
CbrtOperator::~CbrtOperator() {
	delete arg;
}

CosOperator::CosOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
CosOperator::~CosOperator() {
	delete arg;
}

SinOperator::SinOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
SinOperator::~SinOperator() {
	delete arg;
}

TanOperator::TanOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
TanOperator::~TanOperator() {
	delete arg;
}

ExpOperator::ExpOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
ExpOperator::~ExpOperator() {
	delete arg;
}

LogOperator::LogOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
LogOperator::~LogOperator() {
	delete arg;
}

Log10Operator::Log10Operator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
Log10Operator::~Log10Operator() {
	delete arg;
}

ASinOperator::ASinOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
ASinOperator::~ASinOperator() {
	delete arg;
}

ACosOperator::ACosOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
ACosOperator::~ACosOperator() {
	delete arg;
}

ATanOperator::ATanOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
ATanOperator::~ATanOperator() {
	delete arg;
}

SinHOperator::SinHOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
SinHOperator::~SinHOperator() {
	delete arg;
}

CosHOperator::CosHOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
CosHOperator::~CosHOperator() {
	delete arg;
}

TanHOperator::TanHOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
TanHOperator::~TanHOperator() {
	delete arg;
}

CeilOperator::CeilOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
CeilOperator::~CeilOperator() {
	delete arg;
}

FloorOperator::FloorOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
FloorOperator::~FloorOperator() {
	delete arg;
}

FAbsOperator::FAbsOperator(ParserOperator *argument) : ParserOperator(), arg(argument) {
}
FAbsOperator::~FAbsOperator() {
	delete arg;
}


PowOperator::PowOperator(ParserOperator *left, ParserOperator *right) : ParserOperator(), larg(left), rarg(right) {
}
PowOperator::~PowOperator() {
	delete larg;
	delete rarg;
}

