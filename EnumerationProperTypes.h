/*
 * EnumerationProperTypes.h
 *
 *  Created on: Jul 30, 2020
 *      Author: aharonh
 */

#ifndef FUNCTIONALITY_CALIBRATION_ENUMERATIONPROPERTYPES_H_
#define FUNCTIONALITY_CALIBRATION_ENUMERATIONPROPERTYPES_H_
#include <assert.h>
#include "basicTypes/MEtl/string.h"

#include <map>
#include <vector>
#include <list>
#include <iostream>

#include "functionality/calibration/ContainerPropertyType.h"
#include "functionality/calibration/ProperTypes.h"
#include "basicTypes/MEtl/enumGenerator.h"
#include <memory>
#include <limits.h>

class EnumDictionary
{
public:
  const static int          INVALID_VAL = INT_MAX;
  const static  MEtl::string INVALID_STR_VAL;
  typedef std::map<int, MEtl::string> IntToStr;
  typedef IntToStr::const_iterator    IntToStrIt;
  typedef std::map<MEtl::string, int> StrToInt;
  typedef StrToInt::const_iterator    StrToIntIt;
  static void set(std::atomic<EnumDictionary*>& dictionary, const IntToStr& intToStr,bool freeze = true);
  MEtl::string operator[](int val) const;

  //Note . in this function can be malloc issue, because it return the value by reference and another user can clear this object
  //Still this function is necessary. If  you want to get the string value without allocating memory e.g in online after init
  const MEtl::string& operator()(int val) const;
  int   operator[](const MEtl::string& strVal) const;
  void  getListStr(StrContainer& list)const;
  void  getListInts(IntContainer& list)const;
  void  append(const IntToStr& intToStr,bool freeze = false );
  void  reset();
  bool isFreeze() const;
private:
  EnumDictionary():_freeze(false){}
  EnumDictionary(const EnumDictionary& other):_freeze(other._freeze){}
  void operator= (const EnumDictionary& other){}
  void init(const IntToStr& intToStr,bool freeze);
  IntToStr                               _startIntToStr;
  IntToStr                               _intToStr;
  StrToInt                               _strToInt;
  bool                                   _freeze;
};


class OptimizeEnumeration {
  friend OptimizeEnumeration& atot(OptimizeEnumeration& t, const MEtl::string& a);
public:
  OptimizeEnumeration(int val,const EnumDictionary& dictionary);
  virtual ~OptimizeEnumeration();
  OptimizeEnumeration(const OptimizeEnumeration& other);
  virtual const OptimizeEnumeration& operator= (const OptimizeEnumeration& other);

  virtual operator const int() const;
  virtual MEtl::string operator[](int val) const;
  virtual int   operator[](const MEtl::string& strVal) const;
  virtual bool operator == (const OptimizeEnumeration& other) const{return _val == other._val;}
  virtual bool operator != (const OptimizeEnumeration& other) const{return _val != other._val;}
  virtual bool operator < (const  OptimizeEnumeration& other) const{return _val <  other._val;}
  virtual bool operator <= (const OptimizeEnumeration& other) const{return _val <= other._val;}
  virtual bool operator > (const  OptimizeEnumeration& other) const{return _val >  other._val;}
  virtual bool operator >= (const OptimizeEnumeration& other) const{return _val >= other._val;}
protected:
  int          _val;
  const  EnumDictionary& _dictionary;
};

OptimizeEnumeration& atot(OptimizeEnumeration& t, const MEtl::string& a);
MEtl::string ttoa(const OptimizeEnumeration& t);
const char* ttot(const OptimizeEnumeration& t);



#endif /* FUNCTIONALITY_CALIBRATION_ENUMERATIONPROPERTYPES_H_ */
