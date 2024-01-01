#include "PropertiesValidators.h"

typedef	MEtl::string	String;

ExactNamesValidator::ExactNamesValidator	( const String aNames[], const unsigned len )
{
	_InTheRange = ". value must be in the range of ";
	for ( unsigned index = 0; index < len; index++ ) {
		const String& name = aNames[index];
		_InTheRange += index ? " " : "<";
		_InTheRange += name;
		_itemsList.push_back (name);
	}
	_InTheRange += ">";
	_numOfPossibleVals = len;
}

bool	ExactNamesValidator::validate		( const String& key, const String& val, const Properties& container, String& whyNot ) const
{
	for ( auto pName = _itemsList.begin(); pName < _itemsList.end(); pName++ ) {
		if ( *pName == val ) {
			whyNot = "";
			return true;
		}
	}

	whyNot	= "can't set " + key + " to " + val + _InTheRange;
	return false;
}
