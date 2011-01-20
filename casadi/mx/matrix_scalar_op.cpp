/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010 by Joel Andersson, Moritz Diehl, K.U.Leuven. All rights reserved.
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "matrix_scalar_op.hpp"
#include <vector>
#include <sstream>

using namespace std;

namespace CasADi{

MatrixScalarOp::MatrixScalarOp(OPERATION op_, const MX& x, const MX& y) : op(op_){
  setDependencies(x,y);
  setSparsity(x.sparsity());
}

MatrixScalarOp* MatrixScalarOp::clone() const{
  return new MatrixScalarOp(*this);
}

void MatrixScalarOp::print(std::ostream &stream) const{
  stringstream sx; sx << dep(0);
  stringstream sy; sy << dep(1);
  print_c[op](stream,sx.str(),sy.str());
}

void MatrixScalarOp::evaluate(int fsens_order, int asens_order){
  const vector<double>& x = input(0);  // first (possibly non-scalar) argument
  const vector<double>& y = input(1);  // second (scalar) argument
  vector<double>& res = output();
  for(int i=0; i<res.size(); ++i)
    nfun0[op](x[i],y[0],&res[i]);
  
  if(fsens_order>0){
    const vector<double>& dx = fwdSeed(0); // first (possibly non-scalar) argument derivative
    const vector<double>& dy = fwdSeed(1); // second (scalar) argument derivative
    vector<double>& fsens = fwdSens();
    double tmp[3];
    for(int i=0; i<fsens.size(); ++i){
      nfun1[op](x[i],y[0],tmp);
      fsens[i] = tmp[1]*dx[i] + tmp[2]*dy[0]; // chain rule
    }
  }
  
  if(asens_order>0){
    const vector<double>& aseed = adjSeed();
    vector<double>& dx = adjSens(0); // first (scalar) argument derivative
    vector<double>& dy = adjSens(1); // second (possibly non-scalar) argument derivative

    double tmp[3];
    for(int i=0; i<aseed.size(); ++i){
      nfun1[op](x[i],y[0],tmp);
      dx[i] += aseed[i]*tmp[1];
      dy[0] += aseed[i]*tmp[1];
    }
  }
}

} // namespace CasADi

