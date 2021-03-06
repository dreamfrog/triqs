/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2011 by O. Parcollet
 *
 * TRIQS is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * TRIQS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * TRIQS. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#ifndef TRIQS_ARRAYS_MINI_VECTOR_H 
#define TRIQS_ARRAYS_MINI_VECTOR_H 
#include "./first_include.hpp"
#include <iostream>
#include "./compiler_details.hpp"
#include "./exceptions.hpp"
#include <boost/serialization/utility.hpp>
#include <vector>
#include <triqs/utility/tuple_tools.hpp>

#define TRIQS_MINI_VECTOR_NRANK_MAX 10

namespace triqs { namespace utility { 

 template <typename T, int Rank> 
  class mini_vector { 
   T _data[Rank];
   friend class boost::serialization::access;
   template<class Archive> void serialize(Archive & ar, const unsigned int version) { ar & boost::serialization::make_nvp("_data",_data); }
   void init() { for (int i=0;i<Rank; ++i) _data[i] = 0;}
   public : 

   typedef T value_type;

   mini_vector(){init();} 

#define AUX(z,p,unused)  _data[p] = x_##p; 
#define IMPL(z, NN, unused)                                \
   mini_vector (BOOST_PP_ENUM_PARAMS(BOOST_PP_INC(NN), T x_)){ \
    static_assert(Rank-1==NN,"mini_vector : incorrect number of variables in constructor");\
    BOOST_PP_REPEAT(BOOST_PP_INC(NN),AUX,nil) }
   BOOST_PP_REPEAT(TRIQS_MINI_VECTOR_NRANK_MAX , IMPL, nil);
#undef IMPL
#undef AUX

   mini_vector(const mini_vector & x){ *this = x; }
   mini_vector(mini_vector && x){ *this = std::move(x); }
   
   template<typename T2> mini_vector(const mini_vector<T2,Rank> & x){ *this = x; }

   mini_vector(const std::vector<T> & v){ 
    if (v.size()!=Rank)
     TRIQS_RUNTIME_ERROR<< "mini_vector construction : vector size incorrect  : expected "<<Rank<<" got : "<< v.size();
    for (int i=0;i<Rank; ++i)  _data[i] = v[i];
   }

   mini_vector & operator=(const mini_vector & x){ for (int i=0;i<Rank; ++i) _data[i] = x._data[i]; return *this;}
   mini_vector & operator=(mini_vector && x) = default; 

   friend void swap(mini_vector & a, mini_vector & b) { std::swap(a._data, b._data);}

   template<typename T2>
   mini_vector & operator=(const mini_vector<T2,Rank> & x){ for (int i=0;i<Rank; ++i) _data[i] = x[i]; return *this;}
   
   T & operator[](size_t i) { return _data[i];}
   const T & operator[](size_t i) const { return _data[i];}
   
   std::vector<T> to_vector () const { std::vector<T> V(Rank); for (int i=0;i<Rank; ++i)  V[i] = _data[i]; return V; }

   T product_of_elements () const { T res=1; for (int i=0;i<Rank; ++i)  res *= _data[i]; return res; }
 
   T * restrict ptr() { return _data;}
   const T * restrict ptr() const { return _data;}

   std::string to_string() const { 
    std::stringstream fs; 
    fs <<"(";
    for (int i=0;i<Rank; ++i)  fs<<(i==0? "" : " ") << (*this)[i];
    fs<<")";
    return fs.str();
   }

   mini_vector<T, Rank+1> append (T const & x) const { 
    mini_vector<T, Rank+1> res;
    for (int i=0;i<Rank; ++i)  res[i]=_data[i];
    res[Rank] = x;
    return res;
   }

   mini_vector<T, Rank+1> front_append (T const & x) const { 
    mini_vector<T, Rank+1> res;
    for (int i=0;i<Rank; ++i)  res[i+1]=_data[i];
    res[0] = x;
    return res;
   }

   mini_vector<T, Rank-1> pop () const { 
    mini_vector<T, Rank-1> res;
    for (int i=0;i<Rank-1; ++i)  res[i]=_data[i];
    return res;
   }

   mini_vector<T, Rank-1> front_pop () const { 
    mini_vector<T, Rank-1> res;
    for (int i=1;i<Rank; ++i)  res[i-1]=_data[i];
    return res;
   }

   friend std::ostream & operator << ( std::ostream & out, mini_vector const & v ) {return out<<v.to_string();}
   friend std::stringstream & operator << ( std::stringstream & out, mini_vector const & v ) { out<<v.to_string(); return out;}
  }; // class mini_vector

 template <typename T, int R> 
  bool operator ==(mini_vector<T,R> const & v1, mini_vector<T,R> const & v2) {
   for (int i=0;i<R; ++i) { if (v1[i]!=v2[i]) return false;}
   return true;
  }

 template <typename T, int R> bool operator !=(mini_vector<T,R> const & v1, mini_vector<T,R> const & v2) { return (!(v1==v2));}

 template <typename T, int R1, int R2> 
  mini_vector<T, R1+R2> join (mini_vector<T,R1> const & v1, mini_vector<T,R2> const & v2) {
   mini_vector<T, R1+R2> res;
   for (int i=0;i<R1; ++i)  res[i]=v1[i];
   for (int i=0;i<R2; ++i)  res[R1+i]=v2[i];
   return res;
  }

 template <typename T1, typename T2, int Rank> 
  T1 dot_product(mini_vector<T1,Rank> const & v1, mini_vector<T2,Rank> const & v2) {
   T1 res=0;
   for (int i=0;i<Rank; ++i)  res += v1[i]*v2[i];
   return res;
  }

 struct tuple_to_mini_vector_aux { template<typename M, typename V> V * operator()(M const & m,  V * v) { *v = m; return ++v;}};

 // change : the first version crash clang 3.3, but not svn version.
 // must be a bug, corrected since then
/* 
 template<typename T, typename ... U> 
  mini_vector<T,sizeof...(U)> tuple_to_mini_vector(std::tuple<U...> const & t) { 
   mini_vector<T,sizeof...(U)> res;
   triqs::tuple::fold(tuple_to_mini_vector_aux(),t,&res[0]); 
   return res;
  }
*/
 template<typename T, typename TU> 
  mini_vector<T,std::tuple_size<TU>::value> tuple_to_mini_vector(TU const & t) { 
   mini_vector<T,std::tuple_size<TU>::value> res;
   triqs::tuple::fold(tuple_to_mini_vector_aux(),t,&res[0]); 
   return res;
  }

}}//namespace triqs::arrays 
#endif

