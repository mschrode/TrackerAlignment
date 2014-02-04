#ifndef VARIABLE_H
#define VARIABLE_H

#include <exception>
#include <iostream>
#include <vector>

#include "TString.h"


class Variable {
public:
  Variable(const TString &expr);

  TString operator()() const { return label()+unit(); }
  TString label() const { return label_; }
  TString screenLabel() const { return screenLabel_; }
  TString unit() const { return unit_; }

  double min() const;
  double max() const;

  double eval(const std::vector<float> &args) const;
  unsigned int nTreeVariables() const { return treeVariables_.size(); }
  TString treeVariable(unsigned int i) const { return treeVariables_.at(i); }
  

private:
  std::vector<TString> treeVariables_;
  std::vector<TString> functions_;
  std::vector<TString> operators_;
  TString label_;
  TString screenLabel_;
  TString unit_;

  double eval(double x1, const TString &op, const TString &func2, double x2) const;
  double eval(const TString &func, double x) const;
  void splitIntoOperands(const TString &expr, std::vector<TString> &operands, std::vector<TString> &operators) const;
  void splitIntoFunctionAndTreeVariable(const TString &expr, TString &func, TString &treeVar) const;
  void setLabel();
  void setUnit();
  double min(const TString &treeVar) const;
  double max(const TString &treeVar) const;
};


Variable::Variable(const TString &expr) {
  TString str(expr);
  str.ReplaceAll(" ","");
  std::vector<TString> operands;
  splitIntoOperands(str,operands,operators_);
  functions_ = std::vector<TString>(operands.size(),"");
  treeVariables_ = std::vector<TString>(operands.size(),"");
  for(unsigned int i = 0; i < operands.size(); ++i) {
    splitIntoFunctionAndTreeVariable(operands.at(i),functions_.at(i),treeVariables_.at(i));
  }
  if( !( functions_.size() == treeVariables_.size() &&
	 treeVariables_.size() >= 1 &&
	 treeVariables_.size() == operators_.size()+1   ) ) {
    std::cerr << "\n\nERROR in Variable: unrecognised expression '" << expr << "'" << std::endl;
    throw std::exception();
  }
  
  setLabel();
  setUnit();
}


void Variable::setLabel() {
  label_ = "";
  for(unsigned int i = 0; i < treeVariables_.size(); ++i) {
    if( i > 0 ) label_ += operators_.at(i-1);
    if( functions_.at(i) == "1" ) {
      label_ += treeVariables_.at(i);
    } else {
      label_ += functions_.at(i) + "(" + treeVariables_.at(i) + ")";
    }
  }
  screenLabel_ = label_;
  screenLabel_.ReplaceAll("*","");
  screenLabel_.ReplaceAll("(","");
  screenLabel_.ReplaceAll(")","");
  std::cout << "Found Variable '" << screenLabel() << "'" << std::endl;
  label_.ReplaceAll("*"," #upoint ");
  label_.ReplaceAll("d","#Delta ");
  label_.ReplaceAll("phi","#phi");
}


void Variable::setUnit() {
  unit_ = "";
  for(unsigned int i = 0; i < treeVariables_.size(); ++i) {
    const TString var(treeVariables_.at(i));
    TString str("");
    if(      var == "x"  ) str = "cm";
    else if( var == "y"  ) str = "cm";
    else if( var == "z"  ) str = "cm";
    else if( var == "r"  ) str = "cm";
    else if( var == "dx" ) str = "cm";
    else if( var == "dy" ) str = "cm";
    else if( var == "dz" ) str = "cm";
    if( str != "" ) {
      if( unit_ == "" ) unit_ += str;
      else              unit_ += " "+str;
    }
  }
  if( unit_ != "" ) {
    unit_ = " ["+unit_+"]";
  }
}


// Recognises the following expressions
// - <var>
// - <var1> * <var2>
void Variable::splitIntoOperands(const TString &expr, std::vector<TString> &operands, std::vector<TString> &operators) const {
  TString str(expr);
  if( str.Contains("*") ) {
    const int pos = str.First("*");
    operators.push_back("*");
    operands.push_back(str(0,pos));
    operands.push_back(str(pos+1,str.Length()-pos-1));
  } else {
    operands.push_back(str);
  }
}


// Recognises the following expressions:
// - <var>; func is "1"
// - cos(<var>); func is "cos"
// - sin(<var>); func is "sin"
void Variable::splitIntoFunctionAndTreeVariable(const TString &expr, TString &func, TString &treeVar) const {
  TString str(expr);
  if( expr.EndsWith(")") ) {	// Assume some function
    const int pos = str.First("(");
    func = str(0,pos);
    treeVar = str(pos+1,str.Length()-pos-2);
  } else {			// Assume pure tree variable
    func = "1";
    treeVar = str;
  }
}


double Variable::eval(const std::vector<float> &args) const {
  if( args.size() != operators_.size()+1 ) {
    std::cerr << "\n\nERROR in Variable::eval(): wrong number of arguments given" << std::endl;
    throw std::exception();
  }
  double val = eval(functions_.front(),args.front());
  for(unsigned int i = 0; i < operators_.size(); ++i) {
    val = eval(val,operators_.at(i),functions_.at(i+1),args.at(i+1));
  }

  return val;
}


double Variable::eval(double x1, const TString &op, const TString &func, double x2) const {
   double val = x1;
   if( op == "*" ) val *= eval(func,x2);
  
   return val;
}


double Variable::eval(const TString &func, double x) const {
  double val = x;
  if(      func == "cos" ) val = cos(x);
  else if( func == "sin" ) val = sin(x);

  return val;
}


double Variable::min() const {
  double val = -9999.;
  if( treeVariables_.size() == 1 ) {
    val = min(treeVariables_.front());
  }

  return val;
}

double Variable::max() const {
  double val = 9999.;
  if( treeVariables_.size() == 1 ) {
    val = max(treeVariables_.front());
  }

  return val;
}


double Variable::min(const TString &treeVar) const {
  double val = -9999.;
  if(      treeVar == "r" ) val = 0.;
  else if( treeVar == "z" ) val = -310.;
  
  return val;
}

double Variable::max(const TString &treeVar) const {
  double val = 9999.;
  if(      treeVar == "r" ) val = 120.;
  else if( treeVar == "z" ) val = 310.;
  
  return val;
}

#endif
