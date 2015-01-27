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

  double eval(const std::vector<float*> &args) const;
  size_t nTreeVariables() const { return treeVariables_.size(); }
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
  double scaledVal(const double x) const;
  void splitIntoOperands(TString expr, std::vector<TString> &operands, std::vector<TString> &operators) const;
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
  // if( operators_.size() > 0 ) std::cout << "operators_.back() = '" << operators_.back() << "'" << std::endl;
  // std::cout << "operands.front() = '" << operands.front() << "'" << std::endl;
  // std::cout << "operands.back() = '" << operands.back() << "'" << std::endl;
  functions_ = std::vector<TString>(operands.size(),"");
  treeVariables_ = std::vector<TString>(operands.size(),"");
  for(unsigned int i = 0; i < operands.size(); ++i) {
    splitIntoFunctionAndTreeVariable(operands.at(i),functions_.at(i),treeVariables_.at(i));
    // std::cout << i << ": operands = '" << operands.at(i) << "'" << std::endl;
    // std::cout << i << ": functions_ = '" << functions_.at(i) << "'" << std::endl;
    // std::cout << i << ": treeVariables_ = '" << treeVariables_.at(i) << "'" << std::endl;
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
  //  std::cout << "Found Variable '" << screenLabel() << "'" << std::endl;
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
    else if( var == "dx" ) str = "#mum";
    else if( var == "dy" ) str = "#mum";
    else if( var == "dz" ) str = "#mum";
    else if( var == "dr" ) str = "#mum";
    if( str != "" ) {
      if( unit_ == "" ) unit_ += str;
      else              unit_ += " "+str;
    }
    if( i > 0 && var == "dphi" ) unit_ = "#mum"; // hack fro rdphi
  }
  if( unit_ != "" ) {
    unit_ = " ["+unit_+"]";
  }
}


// Recognises the following expressions
// - <var>
// - <var1> * <var2>
void Variable::splitIntoOperands(TString expr, std::vector<TString> &operands, std::vector<TString> &operators) const {
  if( expr.Contains("*") ) {
    const int pos = expr.First("*");
    operators.push_back("*");
    operands.push_back(expr(0,pos));
    operands.push_back(expr(pos+1,expr.Length()-pos-1));
  } else {
    operands.push_back(expr);
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
  //  std::cout << "'" << expr << "' --> '" << func << "' : '" << treeVar << "'" << std::endl;
}


double Variable::eval(const std::vector<float*> &args) const {
  if( args.size() != operators_.size()+1 ) {
    std::cerr << "\n\nERROR in Variable::eval(): wrong number of arguments given" << std::endl;
    throw std::exception();
  }
  double val = eval(functions_.front(),*(args.front()));
  //  std::cout << ">>> val('" << functions_.front() << "','" << args.front() << ") = " << val << std::endl;
  for(unsigned int i = 0; i < operators_.size(); ++i) {
    val = eval(val,operators_.at(i),functions_.at(i+1),*(args.at(i+1)));
  }

  return scaledVal(val);
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


// Scale value to a different unit, e.g. dr is in mu instead of cm
// this is really clumsy and only works for products, deltas
// really should restructure this, making variable a composite or sth
double Variable::scaledVal(const double x) const {
  double scale = 1.;
  for(unsigned int i = 0; i < treeVariables_.size(); ++i) {
    const TString var(treeVariables_.at(i));
    if(      var == "dr"   ) scale *= 1E4; // in mum
    else if( var == "dx"   ) scale *= 1E4; // in mum
    else if( var == "dy"   ) scale *= 1E4; // in mum
    else if( var == "dz"   ) scale *= 1E4; // in mum
    else if( var == "dphi" ) scale *= 1E4; // in murad
  }

  return x*scale;
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
