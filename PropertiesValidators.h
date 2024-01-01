#ifndef __PROPERTIES_VALIDATORS_H__
#define __PROPERTIES_VALIDATORS_H__

#include <ostream>
#include "basicTypes/MEtl/string.h"
#include <map>
#include <algorithm>
#include <sstream> 
#include "functionality/calibration/Properties.h"
#include "functionality/calibration/ItemListValidators.h"
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <cctype>
#include "functionality/calibration/EnumerationProperTypes.h"
#define DUMMY_FINITE_RANGE_VALIDATOR_STR D#F#R#V#F#

enum NumericValidatorSignType { 
  IS_UNSIGNED = false, 
  IS_SIGNED = true// default
  };

// checks if a string consists of alpha character
inline bool checkAlpha(const char* str)
{
  size_t size = strlen(str);
  for (size_t i = 0; i < size; i++){
    if (isalpha(*(str+i)))
      return true;
  }
  return false;
}


// checks if a string consists of "e-" or "e+" or "E-" or "E+" character
inline bool checkExpAlpha(const char* str)
{
  bool expAlphaFound = false;
  size_t size = strlen(str);
  for (size_t i = 0; i < size; i++){
    if (isalpha(*(str+i))) {
      if ( expAlphaFound == true ) {
        return false;
      }
      else if ( (i < size-1)
        && ((*(str+i) == 'E') || (*(str+i) == 'e'))
        && ((*(str+i+1) == '-') || (*(str+i+1) == '+')) ) {
          expAlphaFound = true;
      }
      else {
        return false;
      }
    }
  }

  return expAlphaFound;
}

class FiniteRangeValidator:public DiscreteItemsValidator{
public:
  FiniteRangeValidator(const MEtl::string& range, Properties* prop=NULL);

  virtual bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const;
  
  private:
  MEtl::string _validRange;
  bool _dummyFiniteRangeValidatorFlag;
};

class ExactNamesValidator	: public DiscreteItemsValidator {
public:
	typedef	MEtl::string	String;
	ExactNamesValidator	( const String aNames[], const unsigned len );
	bool validate		( const String& keyname, const String& keyval, const Properties& container, String& whyNot ) const;

protected:
	String		 		_InTheRange;
};





class FileValidator:public DiscreteItemsValidator{
public:
  FileValidator(const MEtl::string& range, Properties* prop=NULL);

  virtual bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const;
  
private:
  MEtl::string _validRange;
  bool _allFilesFlag;
};

class DirectoryValidator:public FileValidator{
public:
  DirectoryValidator(const MEtl::string& range, Properties* prop=NULL);
};

class EnumerationValidator : public Property::Validator
{

public:
  EnumerationValidator(const EnumDictionary& enumDictionary, Properties* prop=NULL): Property::Validator(prop),_enumDictionary(enumDictionary)
  {

  }
  virtual bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot) const {
    whyNot = "";
    if(validateVal(keyval)){
      return true;
    }
    std::ostringstream out;
    StrContainer list;
    _enumDictionary.getListStr(list);
        out << "can't set " << keyname << " to " << keyval<< ". value must be in the range of " << ttoa(list);
    whyNot = out.str().c_str();
    return false;
  }
  virtual bool validateVal(const MEtl::string& keyval) const
  {
    if(!_enumDictionary.isFreeze()){
      return true;
    }
    return (_enumDictionary[keyval]  != EnumDictionary::INVALID_VAL) ;
  }
  const EnumDictionary& _enumDictionary;
};

template <class T>
class RangeValidator:public Property::Validator{
public:
  RangeValidator(const MEtl::string& rangeStr, AllowPairOverlapping allowPairOverlap = DONT_ALLOW_PAIR_OVERLAP, Properties* prop=NULL): Property::Validator(prop),_rangeStr(rangeStr){
    _range._allowPairOverlap = allowPairOverlap;
    atot(_range, rangeStr);
  }

  // We turn this minval and maxval to a Range<T> with 1 pair in it.
  RangeValidator(T minval, T maxval, Properties* prop=NULL): Property::Validator(prop)
  {
    std::pair<T,T> mypair = std::make_pair(minval, maxval);
    _range.push_back(mypair);
    char buf[1024] = {0};
    sprintf(buf, "{%.2f  %.2f}",(double)minval, (double)maxval);
    _rangeStr = buf;
    assert(maxval >= minval);
  }

  virtual bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const{
    whyNot = "";
    T val;
    if (checkAlpha(keyval.c_str()) && !checkExpAlpha(keyval.c_str())){
      char buf[1024] = {0};
      sprintf(buf, "Can't set %s to %s. Value contains illegal alphabetic characters", keyname.c_str(), keyval.c_str());
      whyNot = buf;
      return false;
    }

    if (keyval.empty()){
      char buf[1024] = {0};
      sprintf(buf, "Can't set %s to an empty value", keyname.c_str());
      whyNot = buf;
      return false;
    }

    atot(val, keyval);
    if(_range.isInRange(val)){
      // apply the default validator - if for example T=int and the user used a float then this will invoke an error.
      const Property::Validator* validator = GetDefaultValidator(val);
      if (validator){
        return validator->validate(keyname, keyval, container, whyNot);
      }
      return true;
    }
    else{
      char buffer[1024];
      sprintf(buffer, "Can't set %s to %s. It must be within the following range: %s", keyname.c_str(), keyval.c_str(), _rangeStr.c_str());
      whyNot = buffer;
      return false;
    }
  }

private:
  Range<T> _range;
  MEtl::string _rangeStr;
};

class IntValidator: public RangeValidator<int>{
public: 
  IntValidator(int minval, int maxval, Properties* prop=NULL) : RangeValidator<int>(minval, maxval, prop) {}
};

template <typename T>
class NumericValidator:public Property::Validator{
public:
  NumericValidator(NumericValidatorSignType isSigned = IS_SIGNED, Properties* prop = NULL);
  virtual bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const;
  
private:
  NumericValidatorSignType _isSigned;
};

template<typename T> 
inline NumericValidator<T>::NumericValidator(NumericValidatorSignType isSigned, Properties* prop) : Property::Validator(prop), _isSigned(isSigned)
{
}

template<typename T> 
inline bool NumericValidator<T>::validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const
{
#ifdef PALLADIUM 
  return true;
#else
  whyNot = "";

  // Note: A alphabetic character is not valid - an exponent is also no good.
  // A decimal point with a non zero digit after it is also not valid.
  const char * str;
  str = keyval.c_str();
  size_t size = strlen(str);
  bool isHex = (size>2 && str[0]=='0' && (str[1]=='x' || str[1]=='X')) ? true : false;
  size_t i = isHex ? 2 : 0;
  bool checkForDecimal = false;
  for (; i < size; i++){
    if (isHex){
      if (!isxdigit(*(str+i))){
        break;
      }
    }
    else{
      if (isalpha(*(str+i))){
        break;
      }
    }
    if (checkForDecimal){
      if ((*(str+i))!='0'){
        break;
      }
    }
    else if ((*(str+i))=='.'){
      checkForDecimal = true;
    }
  }

  char buffer[1024];
  if (keyval.empty()){
    sprintf(buffer, "Can't set %s to an empty value", keyname.c_str());
    whyNot = buffer;
    return false;
  }
  if (i == size){  // the value is indeed numeric so now check for variable size visa vis its value

    long long bigint;
    std::stringstream ss(str);
    ss >> bigint;


    if ((sizeof(T) == 1) && (_isSigned? (bigint > 127) : (bigint > 255))){ // signed/unsigned char
      sprintf(buffer, "Can't set %s to %s. %s cannot have a value over %s", keyname.c_str(), keyval.c_str(), (_isSigned?"A char":"An unsigned char"), (_isSigned?"127":"255"));
      whyNot = buffer;
      return false;
    }

    if ((sizeof(T) == 2) && (_isSigned? (bigint > 32767) : (bigint > 65535)) ){  // signed/unsigned short
      sprintf(buffer, "Can't set %s to %s. %s cannot have a value over %s", keyname.c_str(), keyval.c_str(), (_isSigned?"A short":"An unsigned short"), (_isSigned?"32767":"65535"));
      whyNot = buffer;
      return false;
    }

    if ((sizeof(T) == 4) && (_isSigned? (bigint > 2147483647u) : (bigint > 4294967295u))  ){  // signed/unsigned long
      sprintf(buffer, "Can't set %s to %s. %s cannot have a value over %s", keyname.c_str(), keyval.c_str(), (_isSigned?"A long":"An unsigned long"), (_isSigned?"2147483647":"4294967295"));
      whyNot = buffer;
      return false;
    }

    // the numeric value is valid
    return true;
  }
  else{
    sprintf(buffer, "Can't set %s to %s. Must be a non-decimal number without alphabetic characters.", keyname.c_str(), keyval.c_str());
    whyNot = buffer;
    return false;
  }
#endif
}


class CharacterValidator:public Property::Validator{
public:
  inline CharacterValidator(Properties* prop = NULL) : Property::Validator(prop)
  {
  }

  inline bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const
  {
    whyNot = "";

    // Note: if there is more than 1 character - this is not valid.
    const char * str;
    str = keyval.c_str();
    size_t size = strlen(str);
    
    if ((size > 1) || (size==0)){ 
      char buffer[1024];
      sprintf(buffer, "Can't set %s to %s. Value must be 1 character long", keyname.c_str(), keyval.c_str());
      whyNot = buffer;
      return false;
    }
    return true;
  }
};


class BooleanValidator:public DiscreteItemsValidator{
public:
  BooleanValidator(const MEtl::string& pos, const MEtl::string& neg, Properties* prop=NULL);

  virtual bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const;
};

class TrueFalseValidator: public BooleanValidator{
public: 
  TrueFalseValidator(Properties* prop=NULL) : BooleanValidator("true", "false", prop) {}
};

class AlwaysTrueValidator:public DiscreteItemsValidator{
public:
  AlwaysTrueValidator(Properties* prop=NULL): DiscreteItemsValidator(prop)//_prop(prop)
  {
    _numOfPossibleVals = -1;
  }
  virtual bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const{
    return true;
  }
};

template<typename T> 
class RangeSyntaxValidator:public Property::Validator{
public:
  RangeSyntaxValidator(Properties* prop=NULL);
  virtual bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const;
};

template<typename T> 
inline RangeSyntaxValidator<T>::RangeSyntaxValidator(Properties* prop) : Property::Validator(prop){
}

template<typename T> 
inline bool RangeSyntaxValidator<T>::validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const
{
  if (keyval == ""){
    // If there is no value for writeRange ("writeRange=") then this is fine as the user didn't intend to use this anyhow.
    return true;
  }

  Range<T> t;
  char* pchar = (char*)&(keyval.c_str())[0];
  atot((Range<T > &) t, keyval);
  // if range vector is empty this denotes overlapping between pairs
  if (t.empty()){
    char buf[1024] = {0};
    sprintf(buf, "Illegal set of pairs in %s - no overlapping between pairs allowed. You must use 2 dots to denote a range and a comma as a delimiter if you have more than 1 range. e.g. writeRange=3..7,13",keyval.c_str());
    whyNot = buf;
    return false;
  }
  // We check each range pair (where a comma delimits between sets of pairs) and if the pair items are both 0 (which denotes they were uninitialized)
  // then this means that there was a syntax error in the expression. If the user sets the range to 0 (so that the pair items are legitimately both 0)
  // then this is taken care of too below.
  for(unsigned int i = 0; i < t.size(); ++i){
    T first = (T)(((std::pair<T,T>)t[i]).first);
    T second =(T)(((std::pair<T,T>)t[i]).second);
    std::string rangeItemFromKeyVal;
    std::stringstream inputStr(pchar);
    std::getline(inputStr, rangeItemFromKeyVal, ',');
    bool isKeyValZero = (rangeItemFromKeyVal=="0");
    std::string firstOfPair = (char*)ttoa(first).c_str();
    std::string secondOfPair = (char*)ttoa(second).c_str();
    if (((firstOfPair.empty() && secondOfPair.empty()) || (firstOfPair=="0" && secondOfPair=="0")) && !isKeyValZero){
      char buf[1024] = {0};
      sprintf(buf, "Can't set %s to %s. Bad syntax. You must use 2 dots to denote a range and a comma as a delimiter if you have more than 1 range. e.g. writeRange=3..7,13", keyname.c_str(), keyval.c_str());
      whyNot = buf;
      return false;
    }

    pchar = strstr(pchar,",");
    if (pchar){
      pchar += 1;
    }
  }
  return true;
}
/////////////////////////////////////////////////////////////////////////////

inline FiniteRangeValidator::FiniteRangeValidator(const MEtl::string& range, Properties* prop) : DiscreteItemsValidator(prop)
{
  _dummyFiniteRangeValidatorFlag = false;
  if (!range.empty() && range[0]=='{') { 
    Strings s;
    atot(s, range);
    _itemsList = s.data;
    _validRange = range;
    _numOfPossibleVals = (int)s.data.size();
  }
  else {
    atot(_itemsList, range);
    const std::vector<MEtl::string>& enumVals = _itemsList;
    std::ostringstream out;
    out << "{";
    for(size_t i = 0; i<enumVals.size(); ++i){
      out << enumVals[i];
      if(i+1 != enumVals.size()){
        out << ", ";
      }
      if(enumVals[i]=="DUMMY_FINITE_RANGE_VALIDATOR_STR"){
        _dummyFiniteRangeValidatorFlag = true;
      }
    }
    out << "}";
    _validRange = out.str().c_str();
    _numOfPossibleVals = (int)enumVals.size();
  }
}

inline bool FiniteRangeValidator::validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const
{
  whyNot = "";
  if(_dummyFiniteRangeValidatorFlag){
    return true;
  }
  std::vector<MEtl::string> enumVals = _itemsList;
  std::vector<MEtl::string>::const_iterator element = std::find(_itemsList.begin(), _itemsList.end(), keyval);
  bool found = (element != _itemsList.end());
  if(!found){
    std::ostringstream out;
    out << "can't set " << keyname << " to " << keyval<< ". value must be in the range of ";
    out << _validRange;
    whyNot = out.str().c_str();
  }
  return found;
}

inline FileValidator::FileValidator(const MEtl::string& range, Properties* prop) : DiscreteItemsValidator(prop) {
  _allFilesFlag = false;
  if (!range.empty() && range[0]=='{') { 
    Strings s;
    atot(s, range);
    _itemsList = s.data;
    _validRange = range;
    _numOfPossibleVals = (int)s.data.size();
    for (size_t i=0; i<s.data.size(); ++i) {
      if (s.data[i]=="*" || s.data[i]=="...") {
        _allFilesFlag = true;
        break;
      }
    }
  }
  else {
    atot(_itemsList, range);
    const std::vector<MEtl::string>& enumVals = _itemsList;
    std::ostringstream out;
    out << "{";
    for(size_t i = 0; i<enumVals.size(); ++i){
      out << enumVals[i];
      if(i+1 != enumVals.size()){
        out << ", ";
      }
      if(strstr(enumVals[i].c_str(), "*") || enumVals[i]=="..."){
        _allFilesFlag = true;
      }
    }
    out << "}";
    _validRange = out.str().c_str();
    _numOfPossibleVals = (int)enumVals.size();
  }

}

inline bool FileValidator::validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const {
  whyNot = "";
  if(_allFilesFlag){
    return true;
  }
  std::vector<MEtl::string> enumVals = _itemsList;
  std::vector<MEtl::string>::const_iterator element = std::find(_itemsList.begin(), _itemsList.end(), keyval);
  bool found = (element != _itemsList.end());
  if(!found){
    std::ostringstream out;
    out << "can't set " << keyname << " to " << keyval<< ". value must be in the range of ";
    out << _validRange;
    whyNot = out.str().c_str();
  }
  return found;
}


inline DirectoryValidator::DirectoryValidator(const MEtl::string& range, Properties* prop) :FileValidator(range, prop) {
}

inline BooleanValidator::BooleanValidator(const MEtl::string& pos, const MEtl::string& neg, Properties* prop) : DiscreteItemsValidator(prop)
{
  _numOfPossibleVals = 2;
  _itemsList.push_back(pos);
  _itemsList.push_back(neg);
}

inline bool BooleanValidator::validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const
{

  whyNot = "";
  if(_itemsList[0]==keyval || _itemsList[1]==keyval){
    return true;
  }
  char buffer[1024];
  sprintf(buffer, "Can't set %s to %s. Value must be %s or %s", keyname.c_str(), keyval.c_str(), _itemsList[0].c_str(), _itemsList[1].c_str());
  whyNot = buffer;
  return false;
}


class IntsValidator:public RangeValidator<int>{
public:

  IntsValidator(int minval, int maxval, Properties* prop=NULL): RangeValidator<int>(minval, maxval, prop) {  }
  inline bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const
  {
	  if(keyval.empty())
		  return true; // nothing to validate
	  Ints temp(keyval.c_str());
	  for(size_t i=0; i< temp.data.size(); ++i)
		  if(!RangeValidator<int>::validate(keyname, ttoa(temp[i]), container, whyNot))
			  return false;
	  return true;
  }
};

class FloatsValidator:public RangeValidator<float>{
public:

	FloatsValidator(float minval, float maxval, Properties* prop=NULL): RangeValidator<float>(minval, maxval, prop) {  }
  inline bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot)const
  {
	  if(keyval.empty())
		  return true; // nothing to validate
	  Floats temp(keyval.c_str());
	  for(size_t i=0; i< temp.data.size(); ++i)
		  if(!RangeValidator<float>::validate(keyname, ttoa(temp[i]), container, whyNot))
			  return false;
	  return true;
  }
};

class HexStringValidator : public Property::Validator {
public:
  HexStringValidator(Properties* prop = nullptr) : Property::Validator(prop) {}

  bool validate(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container, MEtl::string& whyNot) const override {
    whyNot = "";

    if (keyval == "0") {
      return true;
    }

    if (keyval.size() % 2) {
      whyNot = "Can't set " + keyname + " to " + keyval + ". Value is not a valid hex string (not whole bytes)";
      return false;
    }

    if (!std::all_of(keyval.begin(), keyval.end(), [](unsigned char c){ return std::isxdigit(c); })) {
      whyNot = "Can't set " + keyname + " to " + keyval + ". Value is not a valid hex string";
      return false;
    }

    return true;
  }
};

#endif
