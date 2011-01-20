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

#include "mx_constant.hpp"
#include <cassert>
#include <vector>
#include <algorithm>
#include "../stl_vector_tools.hpp"

using namespace std;

namespace CasADi{

MXConstant::MXConstant(const Matrix<double> &x) : x_(x){
  setSize(x.size1(),x.size2());
}

MXConstant* MXConstant::clone() const{
  return new MXConstant(*this);
}

void MXConstant::print(std::ostream &stream) const{
  stream << x_;
}

void MXConstant::evaluate(int fsens_order, int asens_order){
  copy(x_.begin(),x_.end(),output().begin());
  if(fsens_order>0){
    fill(fwdSens().begin(),fwdSens().end(),0);
  }
}

bool MXConstant::isConstant() const{
  return true;
}


} // namespace CasADi

