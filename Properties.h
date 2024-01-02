/**
 * @file Properties.h
 */

#ifndef __PROPERTIES__H__
#define __PROPERTIES__H__

#include "basicTypes/MEtl/string.h"
#include <assert.h>

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <vector>

#include "ProperTypes.h"

class boolshit;
class PropertiesManager;
class Properties;
class MetaProperties;
class PropertyVerification;

enum VerificationStatus_e {
  E_VERIFICATION_SUCCEEDED = 0,
  E_VERIFICATION_FAILED = 1,
  E_VERIFICATION_INACTIVE = 2,
  E_VERIFICATION_MANUALLY_DEACTIVATED = 3
};

enum BadCmdLineSyntaxPolicy {
  IGNORE_ON_BAD_SYNTAX,
  WARN_ON_BAD_SYNTAX,
  ABORT_ON_BAD_SYNTAX
};
enum InputStringValidityCheckPolicy {
  DONT_CHECK_INPUT_STRING_VALIDITY = 0,
  CHECK_INPUT_STRING_VALIDITY = 1
};

enum ValidatorType {
  DEFAULT_VALIDATOR,
  DISCRETE_ITEMS_VALIDATOR,
  IP_ADDRESS_VALIDATOR
};

/**
 * @class Property
 * @brief Represents a property which consists of a name and a value.
 *
 * Property hold various attributes, including their name, description, flags, loaded status, modification status,
 * and validator. Properties are used to manage the program configurations.
 */
class Property {
public:
  class Validator {
  public:
    Validator(Properties* prop);
    virtual bool validate(const MEtl::string& key, const MEtl::string& val, const Properties& container,
                          MEtl::string& why) const = 0;
    virtual ~Validator() {}
    Properties* _prop;
    virtual int validatorType() const { return DEFAULT_VALIDATOR; }
  };

  /// @brief Enumerates the possible loaded sources of a property.
  enum Loaded {
    NOT_LOADED = 0,     /** < default*/
    FROM_INF = 1 << 0,  /** < from ifstream (file, string etc.)*/
    FROM_ARGS = 1 << 1, /** < from cmdLine*/
    FROM_ENV = 1 << 2,  /** < getenv*/
    FROM_VEC = 1 << 3,
    FROM_USER = 1 << 4,        /** < from user - operator() or calling a set function*/
    FROM_PROJECT = 1 << 5,     /** < from the project*/
    FAILED_LOADED = 1 << 6,    /** < if we fail loading the file we dont want asserts*/
    PRESETS_MODIFIED = 1 << 7, /** < will enable tracking of preset-modified props*/
    PRESETS_MODIFIED_FAILED = 1 << 8,
    PRESETS_OVERWRITTEN = 1 << 9,
    SIZE = 10
  };

  /// @brief Enumerates the possible flags associated with a property.
  enum Flags {
    PERSISTENT = 1 << 0, /** < export to stream always*/
    VOLATILE = 1 << 1,   /** < don't export, unless export volatile is requested*/
    VISIBLE = 1 << 2,    /** < export to stream*/
    HIDDEN = 1 << 3,     /** < export to stream only if loaded, or set value (and persistent)*/

    ALWAYS = 1 << 4,
    IF_NOT_DEFAULT = 1 << 5,
    IF_USER = 1 << 6,
    IF_CALIB = 1 << 7,
    IF_USER_OR_CALIB = 1 << 8,

    FORBIDDEN = 1 << 9,
    DEPRECATED = 1 << 10,
    META_PROPERTY = 1 << 11,
    META_PROPERTY_NON_DEFAULT = 1 << 12, /** < the value of this meta property field was hardcoded*/
    META_PROPERTY_NO_CHANGE = 1 << 13,   /** < the value of this meta property field cannot be changed by cmd line*/

    CHECKSUM = 1 << 14,

    DEFAULT_FLAGS = PERSISTENT | HIDDEN,
    DEFAULT_ME_FLAGS = DEFAULT_FLAGS | IF_CALIB
  };

  /**
   * @brief Validate the property value.
   *
   * This function validates the property value using the Property's validator. It checks whether the property value
   * adheres to the validation criteria defined by the validator. If the validation fails, the corresponding error
   * explanation is returned in the 'why' parameter.
   *
   * @param[in] key The key of the property being validated.
   * @param[in] val The value of the property being validated.
   * @param[in] container The container of properties to which the property belongs.
   * @param[out] why A reference where the error explanation will be stored if validation fails.
   * @return Returns true if the property value is valid, otherwise returns false.
   */
  bool validate(const MEtl::string& key, const MEtl::string& val, const Properties& container, MEtl::string& why) const;

  template<typename T>
  bool defaultProperty(const MEtl::string& var, const T& val);

  void changeContainer(Properties* container, const MEtl::string& var, const MEtl::string& val);
  void changeContainer(Properties* container);

  /**
   * @brief Construct a Property with a custom validator.
   *
   * This constructor creates a Property object with the provided attributes. It allows specifying a custom validator for
   * value validation.
   *
   * @tparam VALIDATOR The type of the custom validator.
   * @param[in] container_ The container of properties to which the property belongs.
   * @param[in] name_ The name of the property.
   * @param[in] desc_ The description of the property.
   * @param[in] flags_ The flags associated with the property.
   * @param[in] tboolshit_ should not be used
   * @param[in] data_ A pointer to associated data.
   * @param[in] validator_ An instance of the custom validator for property value validation.
   * @param[in] mandatory_ A boolean indicating whether the property is mandatory (default: false).
   */
  template<typename VALIDATOR>
  Property(Properties* container_, const char* name_, const char* desc_, int flags_, bool tboolshit_, void* data_,
           const VALIDATOR& validator_, bool mandatory_ = false)
      : _container(container_)
      , name(name_)
      , desc(desc_)
      , flags(flags_)
      , loaded(0)
      , modified(false)
      , tboolshit(tboolshit_)
      , mandatory(mandatory_)
      , _ownsValidator(true)
      , data(data_)
      , _validator(new VALIDATOR(validator_)) {}

  /**
   * @brief Construct a Property.
   *
   * This constructor creates a Property object with the provided attributes.
   *
   * @param[in] container_ The container of properties to which the property belongs.
   * @param[in] name_ The name of the property.
   * @param[in] desc_ The description of the property.
   * @param[in] flags_ The flags associated with the property.
   * @param[in] tboolshit_ Should not be used
   * @param[in] data_ A pointer to associated data.
   * @param[in] validator_ A pointer to a validator for property value validation.
   * @param[in] mandatory_ A boolean indicating whether the property is mandatory (default: false).
   */
  Property(Properties* container_, const char* name_, const char* desc_, int flags_, bool tboolshit_, void* data_,
           const Validator* validator_, bool mandatory_ = false)
      : _container(container_)
      , name(name_)
      , desc(desc_)
      , flags(flags_)
      , loaded(0)
      , modified(false)
      , tboolshit(tboolshit_)
      , mandatory(mandatory_)
      , _ownsValidator(false)
      , data(data_)
      , _validator(validator_) {}

  /**
   * @brief Construct a Property without a validator.
   *
   * This constructor creates a Property object without a validator. The property can still hold attributes such as name,
   * description, flags, and other characteristics, but it won't perform custom value validation.
   *
   * @param[in] container_ The container of properties to which the property belongs.
   * @param[in] name_ The name of the property.
   * @param[in] desc_ The description of the property.
   * @param[in] flags_ The flags associated with the property.
   * @param[in] tboolshit_ Should not be used.
   * @param[in] mandatory_ A boolean indicating whether the property is mandatory (default: false).
   */
  Property(Properties* container_, const char* name_, const char* desc_, int flags_, bool tboolshit_,
           bool mandatory_ = false)
      : _container(container_)
      , name(name_)
      , desc(desc_)
      , flags(flags_)
      , loaded(0)
      , modified(false)
      , tboolshit(tboolshit_)
      , mandatory(mandatory_)
      , _ownsValidator(false)
      , data(nullptr)
      , _validator(nullptr) {}

  /**
   * @brief Destructor for the Property object.
   *
   * The destructor releases any resources associated with the Property object. If the property owns a validator,
   * it is properly deallocated. This ensures that all memory allocated for the property's resources is properly managed.
   */
  virtual ~Property() {
    if (_ownsValidator && _validator) {
      delete _validator;
      _validator = nullptr;
    }
  }

  /**
   * @brief Set a custom validator for property value validation.
   *
   * @tparam VALIDATOR The type of the custom validator.
   * @param[in] validator The custom validator instance to be set.
   */
  template<typename VALIDATOR>
  void setValidator(const VALIDATOR& validator) {
    if (_ownsValidator) {
      delete _validator;
    }
    _ownsValidator = true;
    _validator = new VALIDATOR(validator);
  }

  /**
   * @brief Set a validator for property value validation.
   *
   * @param[in] validator A pointer to the pre-defined validator instance to be set.
   */
  void setValidator(const Validator* validator) {
    if (_ownsValidator) {
      delete _validator;
    }
    _ownsValidator = false;
    _validator = validator;
  }

  /**
   * @brief Get the property's validator along with ownership information.
   *
   * This function retrieves the property's validator along with information about whether the property owns the
   *  validator. The validator is returned as a pointer, and the ownership status is provided in the 'owns' parameter.
   *
   * @param[out] owns A pointer to a boolean where the ownership status of the validator will be stored (if provided).
   * @return A pointer to the property's validator if it owns it, otherwise will return nullptr. If 'owns' is provided,
   * it will indicate whether the property owns the validator.
   */
  const Validator* getValidator(bool* owns) const {
    if (owns) {
      *owns = _ownsValidator;
    }
    return _validator;
  }

protected:
  Properties* _container; /** < Pointer to the container of properties to which this property belongs. */
public:
  const char* const name;      /** < The name of the property. */
  const char* const desc;      /** < The description of the property. */
  unsigned int flags;          /** < Flags associated with the property. */
  mutable unsigned int loaded; /** < Flags indicating the loaded sources of the property's value. */
  mutable bool modified;       /** < Indicates whether the property's value has been modified. */
  bool const tboolshit;
  const bool mandatory;        /** < Indicates whether the property is mandatory. */
  bool _ownsValidator;         /** < Indicates whether the property owns its validator. */
  void* data;                  /** < Pointer to additional data associated with the property. */
  const Validator* _validator; /** < Pointer to the validator for property value validation. */
  virtual void sync(const MEtl::string& val) const = 0;
  bool updated() const;
  virtual const char* type() const = 0;
  virtual MEtl::string ttoaStr() const = 0;

  //************************************
  // Method:    getVerification                 : get the verification object assigned to this property
  // Returns:   PropertyVerification*           : reference to verification object (containing the needed param
  //                                              verifiers and pointers to the verification mechanism)
  //************************************
  virtual PropertyVerification* getVerification() const = 0;
  //************************************
  // Method:    setVerificationPrecision         : set precision level for floating point properties verification
  // Parameter: const double * precisionLevel    : precision level (if NULL the default precision will be used)
  //************************************
  virtual void setVerificationPrecision(const double* precisionLevel) = 0;
  //************************************
  // Method:    requiresVerification  : determines if this Property is safety related and thus requires verification
  //************************************
  virtual bool requiresVerification() const { return NULL != getVerification(); }
  //************************************
  // Method:    verifyVal             : actively verify this property
  // Returns:   VerificationStatus_e  : returns
  //    - E_VERIFICATION_INACTIVE if not initialized or not safety related
  //    - E_VERIFICATION_MANUALLY_DEACTIVATED if safety related but manually deactivated
  //    - E_VERIFICATION_SUCCEEDED/E_VERIFICATION_FAILED according to verification results otherwise
  //************************************
  virtual VerificationStatus_e verifyVal() const = 0;

  //************************************
  // Method:    verifyValIfRequired   : a shortcut for actively verifying this property if it requires verification
  //                                    function returns a boolean value, which unites the various failure enumerations
  // Returns:   true if verification is not required, or if verification is required and succeeded
  //            false if verification is required but failed or inactive (meaning not initialized properly)
  //************************************
  bool verifyValIfRequired() const {
    if (requiresVerification()) {
      return E_VERIFICATION_SUCCEEDED == verifyVal();
    }
    return true;
  }

  //************************************
  // Method:    getLastVerificationStatus : get the last verification result, automatic or "manual"
  //                                        (or E_VERIFICATION_INACTIVE if no verification was done)
  //************************************
  virtual VerificationStatus_e getLastVerificationStatus() const = 0;

  //************************************
  // Method:    deactivateVerification : deactivate verification for the given property
  //************************************
  virtual void deactivateVerification() const = 0;

  virtual const MEtl::string& containerName() const = 0;

  virtual void handleValue(const char* fileName, const char* sectionName) const = 0;

  virtual Properties* getContainer() const = 0;
};

/**
 * @class Properties
 * @brief A collection of properties with the ability to load and read values from different sources,
 *        organized in sections.
 *
 * The Properties class provides a way to manage properties, which are key-value pairs, and supports loading these
 * properties from various sources, including files and strings in sections resolution. It also maintains
 * metadata that can be accessed.
 */
class Properties {
public:
  enum PresetStatus {
    PRESETS_UNINITIALIZED = 0,
    PRESTS_LOADING = 1,
    PRESTS_LOADED = 2
  };

  enum PresetModifiedStatus {
    NO_PRESETS_MODIFIED = 0,
    PRESETS_MODIFIED = 1,
    PRESETS_MODIFIED_FAILED = 2
  };

  virtual ~Properties();

  enum Verbosity {
    SILENT,
    LOW,
    MID,
    HIGH
  };

  enum StoreOptions {
    STORE_DESCRIPTION = 1024,
    STORE_NON_PERSISTENT = 2048,
    STORE_FLAGS = 4096,
    STORE_FREE_PARAMS = 8192,
    STORE_IMPLICIT = 16384,
    STORE_ALL_PERSISTENT = 16384 * 2,
    STORE_REGISTERED_ONLY = 16384 * 4,
    STORE_USAGE = 16384 * 8,
    STORE_CHECKSUM = 16384 * 16,
    STORE_PRESETS_ONLY = 16384 * 32,
    STORE_PEDANTIC = 16384 * 64,
    StoreOptionsSize = 11
  };

  /**
   * @brief Construct a new Properties object
   *
   * @param[in] verbose Verbosity level for messages (default: SILENT)
   * @param[in] out The output stream for printing (default: std::cout)
   * @param[in] err The error stream for printing (default: std::cerr)
   * @param[in] secname The name of the section from which properties will be loaded (default: "")
   * @param[in] presetName The name of the preset (default: "")
   */
  Properties(Verbosity verbose = SILENT, std::ostream& out = std::cout, std::ostream& err = std::cerr,
             const MEtl::string& secname = "", const MEtl::string& presetName = "");

  /**
   * @brief Construct a new Properties object
   *
   * @param[in] secname The name of the section from which properties will be loaded
   * @param[in] flags If the bit corresponding to Property::META_PROPERTY is set, a MetaProperties object will be
   *                  created with the member _metaproperties as NULL. Otherwise, a Properties object will be
   *                  constructed, and its _metaproperties will point to a newly constructed MetaProperties object.
   *                  (default: bit Property::META_PROPERTY is not set).
   * @param[in] presetName The name of the preset (default: "")
   */
  Properties(const MEtl::string& secname, unsigned int flags = 0, const MEtl::string& presetName = "");

  /**
   * @brief Construct a new Properties object
   *
   * @param[in] secname The name of the section from which properties will be loaded
   * @param[in] presetName The name of the preset (default: "")
   * @param[in] flags If the bit corresponding to Property::META_PROPERTY is set, a MetaProperties object will be
   *                  created with the member _metaproperties as NULL. Otherwise, a Properties object will be
   *                  constructed, and its _metaproperties will point to a newly constructed MetaProperties object.
   *                  (default: bit Property::META_PROPERTY is not set).
   */
  Properties(const MEtl::string& secname, const MEtl::string& presetName, unsigned int flags = 0);

  /**
   * @brief Append a new property to the collection if no property with the same key exists.
   *
   * @param[in] property The property to be added.
   */
  void add(Property* property);

  bool load(char** env);

  /**
   * @brief Load properties from the provided vector. Each element in the vector should follow the format: "key=value".
   *        If an invalid string is encountered, such as one with an incorrect format or a key name that doesn't
   *        exist in this Properties object, it will be disregarded, and only valid strings will be loaded.
   *        Invokes onLoaded() and postLoaded() methods.
   *
   * @param[in] A vector of strings, with each string representing a property to be loaded into this Properties object.
   * @return Always returns true.
   */
  bool load(const std::vector<MEtl::string>& args);

  /**
   * @brief Load properties from the provided input stream based on the specified section. If the section name is
   *        nullptr (the default), all properties found in the stream will be loaded. Otherwise, only properties from
   *        the given section will be loaded. Sections are demarcated by lines in the format "[<section name>]".
   *        Each property should appear on a new line in the "key<sep>value" format. Spaces between words are
   *        disregarded. A property is loaded if it exists in this Properties object and its value is deemed valid.
   *        Invokes onLoaded() and postLoaded() methods.
   *
   * @param[in] inStream The input stream containing properties, organized into sections (can be sectionless), following
   *                     the described format.
   * @param[in] sep The character used as the separator between keys and values of properties in the stream.
   * @param[in] section The section from the stream whose properties should be loaded. Default is nullptr.
   * @param[in] source The source of the stream. Optional; default source is Property::FROM_INF.
   * @return true If all properties from the given section were loaded successfully.
   * @return false If loading a property from the section was unsuccessful.
   *
   * @see load(std::istream &inStream, char sep, const char *section = nullptr,
   *        unsigned int source = Property::FROM_INF)
   */
  bool load(std::istream& inStream, char sep, const char* section = nullptr,
            unsigned int source = Property::FROM_INF);

  /**
   * @brief Load properties from the provided input MEtl::string based on the specified section.
   *        For more details @see load(std::istream &inStream, char sep, const char *section = nullptr,
   *        unsigned int source = Property::FROM_INF)
   *
   * @param[in] str The input string containing properties, organized into sections (can be sectionless), following the
   *                described format.
   * @param[in] sep The character used as the separator between keys and values of properties in the stream.
   * @param[in] section The section from the stream whose properties should be loaded. Default is nullptr.
   * @param[in] source The source of the stream. Optional; default source is Property::FROM_INF.
   * @return true If all properties from the given section were loaded successfully.
   * @return false If loading a property from the section was unsuccessful.
   */
  bool load(const MEtl::string& str, char sep, const char* section = nullptr, unsigned int source = Property::FROM_INF);

  /**
   * @brief Load properties from the provided input c string (char*) based on the specified section.
   *        For more details @see load(std::istream &inStream, char sep, const char *section = nullptr,
   *        unsigned int source = Property::FROM_INF)
   *
   * @param[in] str The input string containing properties, organized into sections (can be sectionless), following the
   *                described format.
   * @param[in] sep The character used as the separator between keys and values of properties in the stream.
   * @param[in] section The section from the stream whose properties should be loaded. Default is nullptr.
   * @param[in] source The source of the stream. Optional; default source is Property::FROM_INF.
   * @return true If all properties from the given section were loaded successfully.
   * @return false If loading a property from the section was unsuccessful.
   */
  bool load(const char* str, char sep, const char* section = nullptr, unsigned int source = Property::FROM_INF);

  /**
   * @brief Load properties from the provided group of strings based on the specified section.
   *        For more details @see load(std::istream &inStream, char sep, const char *section = nullptr,
   *        unsigned int source = Property::FROM_INF)
   *
   * @param[in] strings The input strings containing properties, organized into sections (can be sectionless), each string
   *                following the described format.
   * @param[in] sep The character used as the separator between keys and values of properties in the stream.
   * @param[in] section The section from the stream whose properties should be loaded. Default is nullptr.
   * @param[in] source The source of the stream. Optional; default source is Property::FROM_INF.
   * @return true If all properties from the given section were loaded successfully.
   * @return false If loading a property from the section was unsuccessful.
   */
  bool load(const Strings& strings, char sep, const char* section, unsigned int source = Property::FROM_INF);

  /**
   * @brief Load properties from the provided string based on the specified section, without checking their validity.
   *        For more details @see load(std::istream &inStream, char sep, const char *section = nullptr,
   *        unsigned int source = Property::FROM_INF)
   *
   * @param[in] strings The input strings containing properties, organized into sections (can be sectionless), each string
   *                following the described format.
   * @param[in] sep The character used as the separator between keys and values of properties in the stream.
   * @param[in] section The section from the stream whose properties should be loaded. Default is nullptr.
   * @param[in] source The source of the stream. Optional; default source is Property::FROM_INF.
   * @return true If all properties from the given section were loaded successfully.
   * @return false If loading a property from the section was unsuccessful.
   */
  bool loadString(const MEtl::string& str, char sep, const char* section = nullptr,
                  unsigned int source = Property::FROM_INF);

  /**
   * @brief Load properties from argv-argc format based on the specified section. Each string in argv is either a
   *        section name, denoted by "<section name>:" or a property denoted by "<property name>=<property value>".
   *        If the section name is nullptr (the default), will attempt to load all properties from argv. Properties
   *        source will be marked as Property::FROM_ARGS.
   *        Invokes onLoaded() and postLoaded() methods.
   *
   * @param[in] argc The size of argv array.
   * @param[in] argv The array containing all sections and properties.
   * @param[in] section The section from argv whose properties should be loaded. Default is nullptr.
   * @return true If all properties from the given section were loaded successfully.
   * @return false If loading a property from the section was unsuccessful.
   */
  bool load(int argc, char* argv[], const char* section = nullptr);

  /**
   * @brief Load properties from the given file based on the specified section.
   *        For more details @see load(std::istream &inStream, char sep, const char *section = nullptr,
   *        unsigned int source = Property::FROM_INF)
   *
   * @param[in] file The path to the file containing properties, organized into sections (can be sectionless) following
   *                 the described format.
   * @param[in] sep The character used as the separator between keys and values of properties in the file.
   * @param[in] section The section from the file whose properties should be loaded. Default is nullptr.
   * @param[in] source The source of the properties loaded. Optional; default source is Property::FROM_INF.
   * @return true If all properties from the given section were loaded successfully.
   * @return false If loading a property from the section was unsuccessful.
   */
  bool loadFile(const MEtl::string& file, char sep, const char* section = nullptr,
                unsigned int source = Property::FROM_INF);

  /**
   * @brief Load properties from the given file based on the specified section.
   *        For more details @see load(std::istream &inStream, char sep, const char *section = nullptr,
   *        unsigned int source = Property::FROM_INF)
   *
   * @param[in] file The path to the file containing properties, organized into sections (can be sectionless) following
   *                 the described format.
   * @param[in] sep The character used as the separator between keys and values of properties in the file.
   * @param[in] section The section from the file whose properties should be loaded. Default is nullptr.
   * @param[in] source The source of the properties loaded. Optional; default source is Property::FROM_INF.
   * @return true If all properties from the given section were loaded successfully.
   * @return false If loading a property from the section was unsuccessful.
   */
  bool loadFile(const char* file, char sep, const char* section = nullptr, unsigned int source = Property::FROM_INF);

  /**
   * @brief Load properties from the given group of files based on the specified section.
   *        For more details @see load(std::istream &inStream, char sep, const char *section = nullptr,
   *        unsigned int source = Property::FROM_INF)
   *
   * @param[in] multiFile The collection of paths to the files containing properties, organized into sections (can be
   *                      sectionless) following the described format.
   * @param[in] sep The character used as the separator between keys and values of properties in the files.
   * @param[in] section The section from the files whose properties should be loaded. Default is nullptr.
   * @param[in] source The source of the loaded properties. Optional; default source is Property::FROM_INF.
   * @return true If all properties from the given section were loaded successfully.
   * @return false If loading a property from the section was unsuccessful.
   */
  bool loadFile(const Strings& multiFile, char sep, const char* section = nullptr,
                unsigned int source = Property::FROM_INF);

  bool loadCanonical(const std::map<MEtl::string, MEtl::string>& propertiesMap);

  virtual void defaultCalibValues(){};

  /**
   * @brief Stores the properties of this Properties object into a file.
   *        This function saves the properties contained within this Properties object to a specified file. The properties
   *        are stored in the format of "key<sep>value", with each property on a new line. If a section is provided, the
   *        properties will be stored within that section, denoted by "[<section name>]". The function allows customization
   *        through optional parameters for the separator character and flags.
   *        flags is represented as a bitmask, with each bit corresponding to a store option defined in the
   *         @see StorOption enum.
   *
   * @param[in] file The name of the file to which the properties will be saved.
   * @param[in] sep The separator character used between key and value.
   * @param[in] section The section under which properties should be stored (default: nullptr).
   * @param[in] flags Flags indicating the storage behavior (default: Properties::STORE_ALL_PERSISTENT).
   * @return Returns true if the properties were successfully stored, otherwise false.
   */
  bool storeFile(const MEtl::string& file, char sep, const char* section = nullptr,
                 int flags = Properties::STORE_ALL_PERSISTENT);

  /**
   * @brief Stores the properties of this Properties object into a string.
   *        This function saves the properties contained within this Properties object to the provided string.
   *        The properties are stored in the format of "key<sep>value", with each property on a new line. If a section
   *        is provided, the properties will be stored within that section, denoted by "[<section name>]". The function
   *        allows customization through optional parameters for the separator character and mode, which is represented
   *        as a bitmask, with each bit corresponding to a store option defined in the @see StorOption enum.
   *
   * @param[in] data A string to which the properties will be stored.
   * @param[in] sep The separator character used between key and value.
   * @param[in] section The section under which properties should be stored (default: nullptr).
   * @param[in] mode The mode indicating how the properties should be stored
   *                 (default: Properties::STORE_ALL_PERSISTENT).
   * @return Returns true if the properties were successfully stored, otherwise false.
   */
  bool storeString(MEtl::string& data, char sep, const char* section = nullptr,
                   int flags = Properties::STORE_ALL_PERSISTENT);

  /**
   * @brief Retrieves the current status of the section found flag.
   *
   * This method returns the current value of the '_sectionFound' flag, indicating whether
   * the section was found during a previous load operation.
   *
   * @return True if the section was found; false otherwise.
   */
  bool getLastLoadCallSectionFound();

  /**
   * @brief Sets the section found flag to true.
   *
   * This method sets the '_sectionFound' flag to true, indicating that the section was found
   * during a manual load operation or a similar operation bypassing the object's load functions.
   */
  void setLastLoadCallSectionFound();

  /**
   * @brief Resets the section found flag to false.
   *
   * This method resets the '_sectionFound' flag to false, indicating that the section was not found
   * during a manual load operation or a similar operation bypassing the object's load functions.
   */
  void resetLastLoadCallSectionFound();

  /**
   * @brief Validates the mandatory properties in this Properties object.
   *        This function iterates through all the properties in the object. If a property is marked as mandatory,
   *        it ensures that the property has been loaded. If all mandatory properties are loaded, the function returns
   *        true. Otherwise, if any mandatory property is missing, the function returns false and populates the provided
   *        `errMsg` with an error message indicating which mandatory properties are missing.
   *
   * @param[in] errMsg A string that will be populated with an error message if mandatory properties are missing.
   * @return Returns true if all mandatory properties are loaded, otherwise false.
   */
  bool validateMandatory(MEtl::string& errMsg);

  bool validateChecksum(MEtl::string& errMsg, const UInts* checksums);

  /**
   * @brief Set the default separator character for key-value pairs.
   *
   * @param[in] sep The separator character to be set as the default (e.g., '=').
   */
  void setDefaultSeparator(char sep);

  /**
   * @brief Get the default separator character for key-value pairs.
   *
   * @return The default separator character.
   */
  char getDefaultSeparator();
  MEtl::string getTypeName() const;

  /**
   * @brief Set the output stream for standard output messages.
   *        This function allows you to set the output stream where standard messages will be directed. This can be
   *        useful for controlling where the informational and non-error messages are printed.
   *
   * @param[out] out A pointer to the output stream to be set.
   */
  void setOutStream(std::ostream* out) { _pOut = out; }

  /**
   * @brief Get the output stream for standard output messages.
   *        This function retrieves the output stream that is currently set for standard messages.
   *
   * @return A reference to the output stream for standard output messages.
   */
  std::ostream& getOutStream() { return *_pOut; }

  /**
   * @brief Set the output stream for error messages.
   *        This function allows you to set the output stream where error messages will be directed. You can use this to
   *        control the destination of error-related messages.
   *
   * @param[out] err A pointer to the output stream to be set for error messages.
   */
  void setErrStream(std::ostream* err) { _pErr = err; }

  /**
   * @brief Get the output stream for error messages.
   *        This function retrieves the output stream that is currently set for error messages. Error-related messages
   *        will be directed to this stream.
   *
   * @return A reference to the output stream for error messages.
   */
  std::ostream& getErrStream() { return *_pErr; }

  bool loadIdenticalProperties(const Properties& properties);// load only identical properties

  const char* desc(const MEtl::string& key) const;

  /**
   * @brief Validates whether the given value is valid for the specified property by invoking the `validate` method
   *        of the corresponding property, as defined in the class Property.
   *
   * @param[in] key The key of the property for which the value is being validated.
   * @param[in] val The value to be validated.
   * @param[out] whynot A string that will be populated with an explanation if the value is invalid.
   * @return Returns true if the value is valid for the property, otherwise false.
   */
  bool validate(const MEtl::string& key, const MEtl::string& val, MEtl::string& whynot) const;

  int presetModified() { return _modifiedPresets; }

  /**
   * @brief Check if properties have been loaded into the Properties object.
   *
   * @return Returns true if properties have been loaded, otherwise returns false.
   */
  bool loaded() const { return (_loaded ? true : false); }

  bool getValues(const MEtl::string& key, std::vector<MEtl::string>& values, int& numOfPossibleVals) const;

  void* data(const MEtl::string& key) const;

  unsigned int defaultPropertyFlags() const { return _defaultPropertyFlags; }

  /**
   * @brief Get the value of a property and optional validation.
   *
   * This template function retrieves the value of the property with the given key. If the property exists, its value is
   * converted and returned as the template type T. If the property doesn't exist, the provided default value is
   * returned. The function can also perform optional validation on the property's value using the specified validator,
   * and return the default value if the validation fails.
   *
   * @tparam T The type to which the property value will be converted.
   * @param[in] var The key of the property to retrieve.
   * @param[in] defaultVal The default value to return if the property doesn't exist.
   * @param[in] pexist A pointer to a bool indicating if the property exists (default: nullptr).
   * @param[in] validator A pointer to a validator for optional value validation (default: nullptr).
   * @return The property's value of type T, or the default value if the property is not found or validation fails.
   */
  template<typename T>
  T getProperty(const MEtl::string& var, const T& defaultVal, bool* pexist = nullptr,
                const Property::Validator* validator = nullptr) const {

#ifdef CHECK_LOADED_PROPERTY
    assert(_loaded != 0);
#endif
    const char* valString = _getProperty(var);
    if (pexist) {
      *pexist = (valString != nullptr);
    }
    // enable validation in order to return defaultVal not atot default
    if (validator && valString) {
      MEtl::string errorStr;
      bool valid = validator->validate(var, valString, *this, errorStr);
      if (!valid) {
        return defaultVal;
      }
    }
    if (valString) {
      T nonConstDefaultVal(defaultVal);
      return atot(nonConstDefaultVal, MEtl::string(valString));
    }
    return defaultVal;
  }

  MEtl::string getProperty(const MEtl::string& var, const char* defaultVal, bool* pexist = nullptr) const {
#ifdef CHECK_LOADED_PROPERTY
    assert(_loaded != 0);
#endif
    return getProperty(var, MEtl::string(defaultVal), pexist);
  }

  /**
   * @brief Get the value of a property as a C-style string.
   *
   * This function retrieves the value of the property with the given key as a C-style string (const char*). If the
   * property exists, its value is returned as a C-style string. If the property doesn't exist, the function returns
   * nullptr.
   *
   * @param[in] key The key of the property to retrieve.
   * @return The property's value as a C-style string, or nullptr if the property is not found.
   */
  const char* getProperty(const char* key) const {
    return _getProperty(key);
  }

  /**
   * @brief Set the value of a property.
   *
   * This template function allows you to set the value of a property with the specified key. The provided value is
   * converted to a string using the `ttoa` function and then stored as the property's value. The optional `flags`
   * parameter determines the source of the property value. The function performs validation using the property's
   * `validate` method before setting the value.
   *
   * @tparam T The type of the value to be set.
   * @param[in] var The key of the property to set.
   * @param[in] val The value to be set.
   * @param[in] flags The flags indicating the source of the property value (default: Property::FROM_USER).
   * @return Returns true if the value was successfully set, otherwise false.
   */
  template<typename T>
  bool setProperty(const MEtl::string& var, const T& val, int flags = Property::FROM_USER) {
    MEtl::string valString = ttoa(val);
    bool ok = _setProperty(var, valString, flags);
#ifdef TEST_SET_PROPERTY
    bool exist;
    T curr = getProperty(var, val, &exist);
    bool equal = PracticallyEqual(val, curr);
    if (!equal) {
      _err << "WARNING: value for " << var << " was not set properly (";
      _err << val << " -> " << curr << ")\n";
    }
#endif
    return ok;
  }

  /**
   * @brief Set the value of a property using a 'ttoa' string.
   *
   * This function allows you to set the value of a property with the specified key using a ttoa (template to ASCII)
   * string. The ttoa string is assumed to be a valid representation of the value's type. The property's value is
   * updated with the provided ttoa string, and the source of the property value is marked as user input.
   *
   * @param[in] var The key of the property to set.
   * @param[in] ttoaStr The ttoa string representing the value to be set.
   * @return Returns true if the value was successfully set, otherwise false.
   */
  bool setPropertyFromTtoa(const MEtl::string& var, const MEtl::string& ttoaStr) {
    return _setProperty(var, ttoaStr, Property::FROM_USER);
  }

  const std::map<MEtl::string, MEtl::string>& properties() const { return _map; }
  friend std::ostream& operator<<(std::ostream& out, const Properties& properties);
  void cut(std::vector<MEtl::string>& args);
  void add(const std::vector<MEtl::string>& args);

  /**
   * @brief Set the verbosity level for messages.
   *
   * This function allows you to set the verbosity level for messages generated by the Properties object. The verbosity
   * level determines the amount of detail and information displayed in the messages.
   *
   * @param[in] verbose The verbosity level to be set.
   */
  void setVerbose(Verbosity verbose) { _verbose = verbose; }

  /**
   * @brief Get the current verbosity level for messages.
   *
   * This function retrieves the current verbosity level that has been set for the Properties object. The verbosity
   * level determines the amount of detail and information displayed in the messages.
   *
   * @return The current verbosity level for messages.
   */
  Verbosity getVerbose() const { return _verbose; }

  void inspectUnknownFields(const MEtl::string& key, const MEtl::string& val, unsigned int source,
                            std::vector<std::pair<MEtl::string, MEtl::string>>& unknownFields);

  void handleUnknownFields(unsigned int source, std::vector<std::pair<MEtl::string, MEtl::string>>& unknownFields);
  void updateMetaPropertiesData();
  void updateCmdLineSectionList();
  void checkAndHandleObjectsWithSameSectionName(MEtl::string& key);

  /**
   * @brief Set the section name for loading and storing properties.
   *
   * This function allows you to set the section name to be used for loading and storing properties if a section name
   * was not explicitly provided to the store and load functions. The function handles the case where the provided
   * section name lacks square brackets by adding them internally. Empty section names are accepted. The function
   * provides verbose output to indicate changes in the section name.
   *
   * @param[in] name The section name to be set.
   * @param[in] why A description of why the section name is being set.
   */
  void setName(const MEtl::string& name, const char* why = "");

  /**
   * @brief Set the file name associated with the Properties.
   *
   * @param[in] name The file name to be associated with the `Properties` object.
   */
  void setFileName(const MEtl::string& name) { _fileName = name; }

  /**
   * @brief Get the section name associated with the Properties.
   *
   * @return The section name associated with the `Properties` object.
   */
  const MEtl::string& getName() const { return _name; }

  /**
   * @brief Saves the contents of this Properties object to the provided output stream based on the specified mode.
   *        The mode is represented as a bitmask, with each bit corresponding to a store option defined in the
   *         @see StorOption enum.
   *
   * @param[in] out The output stream to which the Properties will be saved.
   * @param[in] mode The mode indicating how the Properties should be saved.
   * @param[in] sep The separator used between the key and value in the output.
   */
  virtual void store(std::ostream& out, int mode, char sep) const;

  /**
   * @brief Clear the modified status of all properties.
   *
   * This function marks all properties within the Properties object as unmodified.
   */
  void clearModified();

  /**
   * @brief Get a map of modified properties and their original values.
   *
   * This function retrieves a map containing all the properties that have been modified within the `Properties` object.
   * The map pairs the property's key with its value before it was modified. The original values can provide insight
   * into the changes made to the properties.
   *
   * @return A map of modified properties and their original values.
   */
  const std::map<MEtl::string, MEtl::string>& modified() const { return _modified; }

  /**
   * @brief Check if a property with the specified key has been modified after loading.
   *
   * This function checks whether a property with the given key has been modified since it was loaded into the
   * `Properties` object. If the property has been modified, the function returns true; otherwise, it returns false.
   *
   * @param[in] key The key of the property to check for modification.
   * @return Returns true if the property has been modified after loading, otherwise returns false.
   */
  bool isModified(const MEtl::string& key) const;

  /**
   * @brief Find a property by its key.
   *
   * This function searches for a property within the `Properties` object based on the provided key. If a property with
   * the given key exists, a pointer to the corresponding `Property` object is returned. If the property is not found,
   * nullptr is returned.
   *
   * @param[in] key The key of the property to search for.
   * @return A pointer to the `Property` object with the given key, or nullptr if not found.
   */
  const Property* findProperty(const MEtl::string& key) const;

  const std::list<const Property*>& properTies() const { return _properTies; }
  void sync();

  void deactivatePropsVerification() const;

  const std::vector<std::pair<MEtl::string, MEtl::string>>& rejectedFields() const { return _rejectedFields; }

  virtual void validateValues() {}
  MetaProperties* _metaProperties;
  virtual const char* getClassName() { return nullptr; }

  // properties manager stuff
  friend class PropertiesManager;
  void setPropertiesManager(PropertiesManager* propertiesManager);
  virtual bool verifyAllProps(const bool stopOnFirstError = false);
  // override only for the cases of properties with non-unique section name
  virtual const MEtl::string& getPresetName() const { return _presetName.empty() ? getName() : _presetName; }
  void reloadPresets();
  void updatePresets();

protected:
  unsigned int _loaded; /** < Flag indicating whether properties have been loaded. */
  PropertiesManager* _propertiesManager;

private:
  bool validate(const MEtl::string& str, char sep) const;
  bool validate(std::istream& in, char sep) const;
  virtual int store3(char* buf, int sizeOfBuf, char sep) const;

  int _loadPresets;
  int _modifiedPresets;
  std::vector<std::pair<MEtl::string, MEtl::string>> _rejectedFields;

  friend class Property;
  Properties(const Properties& other);
  Properties& operator=(const Properties& other);
  const char* _getProperty(const MEtl::string& var) const;

protected:
  bool _setProperty(const MEtl::string& var, const MEtl::string& val, unsigned int loaded);

private:
  void falsifyBoolshits(unsigned int source = Property::FROM_INF);
  virtual bool validate(const Property* property, const MEtl::string& key, const MEtl::string& val,
                        MEtl::string& whyNot) const {
    return true;
  }

  virtual void onRejected(const Property* property, const MEtl::string& key, const MEtl::string& from,
                          const MEtl::string& to, MEtl::string& whyNot) {}

  virtual void onModified(const Property* property, const MEtl::string& key, const MEtl::string& from,
                          const MEtl::string& to) {}

protected:
  virtual void onLoaded() {}

private:
  virtual void exec(const char* command);
  void postLoaded();
  std::map<MEtl::string, MEtl::string> _map;
  std::map<MEtl::string, int> _unformatted;
  void add(std::vector<MEtl::string>& args, bool cut);
  InputStringValidityCheckPolicy _checkInputStringValidity;

protected:
  Verbosity _verbose;  /** < The verbosity level for messages.*/
  std::ostream& _out;  /** < The output stream for messages.*/
  std::ostream& _err;  /** < The error stream for messages.*/
  std::ostream* _pOut; /** < An alternative output stream.*/
  std::ostream* _pErr; /** < An alternative error stream.*/
private:
  bool _sectionFound;// information about most recent load() call

  std::map<MEtl::string, const Property*> _metaData;
  std::list<const Property*> _properTies;
  MEtl::string _name;     /** < The section name used for loading and storing properties.*/
  MEtl::string _fileName; /** < The associated file name for the properties.*/
  std::map<MEtl::string, MEtl::string>
      _modified; /** < A map storing modified property values before they were modified.*/
  unsigned int _defaultPropertyFlags;
  std::list<MEtl::string> _pendingExec;

protected:
  void setRejected(const MEtl::string& keyname, const MEtl::string& keyval, const Properties& container,
                   const MEtl::string& whyNot);
  void setPropertyPresetModified(const Property* property, bool succeededLoadingPropVal);
  Properties* _me;
  char _defaultSeparator; /** < The default separator used for key-value pairs in the properties.*/
  MEtl::string _presetName;
};

class boolshit {
public:
  explicit boolshit(bool shit)
      : _shit(shit) {}
  void setVal(bool val) { _shit = val; }
  operator bool() const { return _shit; }

private:
  bool _shit;
};

template<typename T>
inline void makeBoolshit(bool& tboolshit, const T& val) {
  tboolshit = false;
}
template<>
inline void makeBoolshit(bool& tboolshit, const boolshit& val) {
  assert(val == false);// boolshits can't have negative default value
  tboolshit = true;
}

//  ******************************************
//      class DefaultVerifier
//  a "do nothing" template class for non-safety related parameters (all required verification functions have empty
//  implementation)
//  ******************************************
class DefaultVerifier {
public:
  VerificationStatus_e getLastStatus() const { return E_VERIFICATION_INACTIVE; }

protected:
  void setPrecision(const double* precisionLevel) {}
  void syncVerifiers(const char* newSectionName) const {}
  PropertyVerification* getPropVerification() const { return nullptr; };
  template<typename T>
  void handleValue(const char* fileName, const char* sectionName, const char* keyName, const T& val) const {}

  template<typename T>
  VerificationStatus_e verify(T& val) const { return E_VERIFICATION_INACTIVE; }

  template<typename T>
  VerificationStatus_e verifyAuto(T& val) const { return E_VERIFICATION_INACTIVE; }

  void deactivateVerification() const {}
};

/**
 * @class ProperT
 * @brief A template implementation of the Property class.
 *
 * ProperT is encapsulating the property binary value that will be translated back and forth from the Property string
 * value using ttoa/atot functions.
 *
 * Template arguments:
 * typename T         : The template type of the binary value
 * class VerifierT    : The parent template class encapsulating the verification method of the property binary value
 *                      (checking bit flips, memory overruns etc) for non-safety related Properties - we will use the
 *                      DefaultVerifier class (also the default template value), which present a low cost "do nothing
 *                      verification for safety-related Properties - the PropertyVerifier class should be used,
 *                      containing all verification info and high cost mechanism.
 */
template<typename T, class VerifierT = DefaultVerifier>
class ProperT : public Property, public VerifierT {
public:
  /**
   * @brief Constructor for ProperT with a custom validator.
   *
   * This constructor creates a ProperT instance with the provided attributes and a custom validator for value
   * validation.
   *
   * @tparam VALIDATOR The type of the custom validator.
   * @param[in] container Pointer to the container of properties to which this property belongs.
   * @param[in] defaultVal The default value of the property.
   * @param[in] name The name of the property.
   * @param[in] desc The description of the property.
   * @param[in] flags Flags associated with the property.
   * @param[in] data Pointer to associated data.
   * @param[in] validator An instance of the custom validator for property value validation.
   * @param[in] mandatory Indicates whether the property is mandatory (default: false).
   */
  template<typename VALIDATOR>
  ProperT(Properties* container, const T& defaultVal, const char* name, const char* desc, int flags, void* data,
          const VALIDATOR& validator, bool mandatory = false)
      : Property(container, name, desc, flags, false, data, validator, mandatory)
      , _val(defaultVal)
      , _defaultVal(defaultVal) {
    makeBoolshit<T>((bool&)tboolshit, defaultVal);
    _container->add(this);
    defaultProperty(name, defaultVal);
  }

  /**
   * @brief Constructor for ProperT.
   *
   * This constructor creates a ProperT instance with the provided attributes.
   *
   * @param[in] container Pointer to the container of properties to which this property belongs.
   * @param[in] defaultVal The default value of the property.
   * @param[in] name The name of the property.
   * @param[in] desc The description of the property.
   * @param[in] flags Flags associated with the property.
   * @param[in] data Pointer to associated data.
   * @param[in] validator Pointer to the pre-defined validator for property value validation.
   * @param[in] mandatory Indicates whether the property is mandatory (default: false).
   */
  ProperT(Properties* container, const T& defaultVal, const char* name, const char* desc, int flags, void* data,
          const Validator* validator, bool mandatory = false)
      : Property(container, name, desc, flags, false, data, validator, mandatory)
      , _val(defaultVal)
      , _defaultVal(defaultVal) {
    makeBoolshit<T>((bool&)tboolshit, defaultVal);
    _container->add(this);
    defaultProperty(name, defaultVal);
  }

  /**
   * @brief Constructor for ProperT without a validator.
   *
   * This constructor creates a ProperT instance without a validator. The property can still hold attributes such as
   * name, description, and flags, but it won't perform custom value validation.
   *
   * @param[in] container Pointer to the container of properties to which this property belongs.
   * @param[in] defaultVal The default value of the property.
   * @param[in] name The name of the property.
   * @param[in] desc The description of the property.
   * @param[in] flags Flags associated with the property.
   * @param[in] mandatory Indicates whether the property is mandatory (default: false).
   */
  ProperT(Properties* container, const T& defaultVal, const char* name, const char* desc, int flags,
          bool mandatory = false)
      : Property(container, name, desc, flags, false, mandatory)
      , _val(defaultVal)
      , _defaultVal(defaultVal) {
    makeBoolshit<T>((bool&)tboolshit, defaultVal);
    _container->add(this);
    defaultProperty(name, defaultVal);
  }

  /// @brief Set the property value to its default value.
  void setDefault() { this->operator()(_defaultVal); }

  /**
   * @brief Set the property value using the assignment operator.
   *
   * @param[in] val The value to set the property to.
   * @return True if the property value was successfully set, false otherwise.
   */
  virtual bool operator()(const T& val) {
    return this->_container->setProperty(this->name, val);
  }

  /**
   * @brief Validate if a value is valid for the property.
   *
   * This function validates whether a provided value is valid for the property, using the property's name and
   * container.
   *
   * @param[in] val The value to validate.
   * @param[out] whyNot A string indicating why the value is not valid (if applicable).
   * @return True if the value is valid for the property, false otherwise.
   */
  bool valid(const T& val, MEtl::string& whyNot) const {
    return validate(name, ttoa(val), *_container, whyNot);
  }

  virtual void sync(const MEtl::string& val) const {
    atot(((T&)_val), val);
    VerifierT::syncVerifiers(_container->getName().c_str());
  }

  /**
   * @brief Get the name of the container to which the property belongs.
   *
   * @return The name of the container.
   */
  virtual const MEtl::string& containerName() const override {
    return _container->getName();
  }

  virtual const char* type() const {
    return ttot(((T&)_val));
  }

  /**
   * @brief Convert the property value to a string.
   *
   * @return A string representation of the property value.
   */
  virtual MEtl::string ttoaStr() const {
    return ttoa(((T&)_val));
  }

  /**
   * @brief Get the property value.
   *
   * This function retrieves the property value. If 'hasValue' is provided, it indicates whether the property has a
   * non-default value. If the property is marked as loaded or modified, it's considered to have a non-default value.
   *
   * @param[out] hasValue A pointer to a boolean where the result will be stored (optional).
   * @return The property value.
   */
  const T& operator()(bool* hasValue = nullptr) const {
#ifdef CHECK_LOADED_PROPERTY
    assert(_container->loaded() != 0);
#endif
    if (hasValue) {
      //*hasValue = (loaded || modified);
      if (NOT_LOADED != loaded || true == modified) {
        *hasValue = true;// hasValue==true ==> NOT default value
      }
      else {
        *hasValue = false;
      }
    }
    // verify the _val binary value if set for automatic verification (usually auto for application properties and manual for brain properties)
    // note that for non-safety related parameters (compiled with the DefaultVerifier class) this function does nothing
    VerifierT::verifyAuto(_val);
    return _val;
  }

  // implementation of all verification functions is via the VerifierT template class

  virtual void setVerificationPrecision(const double* precisionLevel) { VerifierT::setPrecision(precisionLevel); }

  virtual PropertyVerification* getVerification() const { return VerifierT::getPropVerification(); }

  virtual VerificationStatus_e getLastVerificationStatus() const { return VerifierT::getLastStatus(); }

  virtual VerificationStatus_e verifyVal() const { return VerifierT::verify(_val); }

  virtual void deactivateVerification() const {
    VerifierT::deactivateVerification();
  }

  virtual void handleValue(const char* fileName, const char* sectionName) const override {
    VerifierT::handleValue(fileName, sectionName, name, _val);
  }

  /**
   * @brief Get the container to which the property belongs.
   *
   * @return Pointer to the container of properties.
   */
  virtual Properties* getContainer() const override {
    return _container;
  }

protected:
  T _val;              /** < The current value of the property. */
  const T _defaultVal; /** < The default value of the property. */
};

template<typename T>
inline const Property::Validator* GetDefaultValidator(const T& var) {
  return (Property::Validator*)nullptr;
}

// I had to create a special template function for <signed char> and a function called GetSignedCharValidator() -
// as GetDefaultValidator(const signed char &var) did not compile due to a clash with GetDefaultValidator(const unsigned char &var).
const Property::Validator* GetSignedCharValidator();
template<>
inline const Property::Validator* GetDefaultValidator<signed char>(const signed char& var) {
  return GetSignedCharValidator();
}

// There is a need to define the GetDefaultValidator functions here in the *.h file. Otherwise these will not be seen
// at run time - at least in release mode. The implementation is in the *.cpp file as we do not want to include PropertiesValidator.h here.
const Property::Validator* GetDefaultValidator(const unsigned long long& var);
const Property::Validator* GetDefaultValidator(const long long& var);
const Property::Validator* GetDefaultValidator(const unsigned long& var);
const Property::Validator* GetDefaultValidator(const long& var);
const Property::Validator* GetDefaultValidator(const unsigned int& var);
const Property::Validator* GetDefaultValidator(const unsigned short& var);
const Property::Validator* GetDefaultValidator(const int& var);
const Property::Validator* GetDefaultValidator(const short& var);
const Property::Validator* GetDefaultValidator(const unsigned char& var);
const Property::Validator* GetDefaultValidator(const char& var);

/**
 * @class RWProperT
 * @brief Represents a read-write property of a templated type.
 *
 * This class extends the ProperT class, providing read and write access to a property of type T. It allows setting and
 * retrieving property values and inherits validation and other related functionality from ProperT.
 *
 * @tparam T The type of the property's value.
 * @tparam VerifierT The type of verifier used for property verification (default: DefaultVerifier).
 */
template<typename T, class VerifierT = DefaultVerifier>
class RWProperT : public ProperT<T, VerifierT> {
public:
  /**
   * @brief Constructor for RWProperT with a custom validator.
   *
   * This constructor creates an RWProperT instance with the provided attributes and a custom validator for value
   * validation.
   *
   * @tparam VALIDATOR The type of the custom validator.
   * @param[in] container Pointer to the container of properties to which this property belongs.
   * @param[in] defaultVal The default value of the property.
   * @param[in] name The name of the property.
   * @param[in] desc The description of the property.
   * @param[in] flags Flags associated with the property.
   * @param[in] data Pointer to associated data.
   * @param[in] validator An instance of the custom validator for property value validation.
   * @param[in] mandatory Indicates whether the property is mandatory (default: false).
   */
  template<typename VALIDATOR>
  RWProperT(Properties* container, const T& defaultVal, const char* name, const char* desc, int flags, void* data,
            const VALIDATOR& validator, bool mandatory = false)
      : ProperT<T, VerifierT>(container, defaultVal, name, desc, flags, data, validator, mandatory) {}

  /**
   * @brief Constructor for RWProperT.
   *
   * This constructor creates an RWProperT instance with the provided attributes.
   *
   * @param[in] container Pointer to the container of properties to which this property belongs.
   * @param[in] defaultVal The default value of the property.
   * @param[in] name The name of the property.
   * @param[in] desc The description of the property.
   * @param[in] flags Flags associated with the property.
   * @param[in] data Pointer to associated data.
   * @param[in] validator Pointer to the pre-defined validator for property value validation.
   * @param[in] mandatory Indicates whether the property is mandatory (default: false).
   */
  RWProperT(Properties* container, const T& defaultVal, const char* name, const char* desc, int flags, void* data,
            const Property::Validator* validator, bool mandatory = false)
      : ProperT<T, VerifierT>(container, defaultVal, name, desc, flags, data, validator, mandatory) {}

  /**
   * @brief Constructor for RWProperT without a validator.
   *
   * This constructor creates an RWProperT instance without a validator. The property can still hold attributes such as
   * name, description, and flags, but it won't perform custom value validation.
   *
   * @param[in] container Pointer to the container of properties to which this property belongs.
   * @param[in] defaultVal The default value of the property.
   * @param[in] name The name of the property.
   * @param[in] desc The description of the property.
   * @param[in] flags Flags associated with the property.
   * @param[in] mandatory Indicates whether the property is mandatory (default: false).
   */
  RWProperT(Properties* container, const T& defaultVal, const char* name, const char* desc, int flags,
            bool mandatory = false)
      : ProperT<T, VerifierT>(container, defaultVal, name, desc, flags, nullptr, GetDefaultValidator(defaultVal),
                              mandatory) {}

  /**
   * @brief Set the property value using the assignment operator.
   *
   * @param[in] val The value to set the property to.
   * @return True if the property value was successfully set, false otherwise.
   */
  bool operator()(const T& val) {
    return this->_container->setProperty(this->name, val);
  }

  /**
   * @brief Get the property value.
   *
   * This function retrieves the property value. If 'hasValue' is provided, it indicates whether the property has a
   * non-default value. If the property is marked as loaded or modified, it's considered to have a non-default value.
   *
   * @param[out] hasValue A pointer to a boolean where the result will be stored (optional).
   * @return The property value.
   */
  const T& operator()(bool* hasValue = nullptr) const {
    return ProperT<T, VerifierT>::operator()(hasValue);
  }
};

// We specialize the RWProperT template for bool - so that we can control what the value of
// an input of type integer is - in the case where the user did not use a bool. Note: we cannot specialize
// just 1 function of the template class but have to redo the whole class.
template<>
class RWProperT<bool> : public ProperT<bool> {
public:
  template<typename VALIDATOR>
  RWProperT(Properties* container, const bool& defaultVal, const char* name, const char* desc, int flags, void* data,
            const VALIDATOR& validator, bool mandatory = false)
      : ProperT<bool>(container, defaultVal, name, desc, flags, data, validator, mandatory) {}

  RWProperT(Properties* container, const bool& defaultVal, const char* name, const char* desc, int flags, void* data,
            const Property::Validator* validator, bool mandatory = false)
      : ProperT<bool>(container, defaultVal, name, desc, flags, data, validator, mandatory) {}

  RWProperT(Properties* container, const bool& defaultVal, const char* name, const char* desc, int flags,
            bool mandatory = false)
      : ProperT<bool>(container, defaultVal, name, desc, flags, nullptr, GetDefaultValidator(defaultVal), mandatory) {}

  bool operator()(const int& val) {
    bool boolVal = true;
    if (val <= 0) {
      boolVal = false;
    }
    return this->_container->setProperty(this->name, boolVal);
  }

  bool operator()(const bool& val) {
    return this->_container->setProperty(this->name, val);
  }
  const bool& operator()(bool* hasValue = nullptr) const {
    return ProperT<bool>::operator()(hasValue);
  }
};

template<typename T>
bool Property::defaultProperty(const MEtl::string& var, const T& val) {
  return _container->setProperty(var, val, Property::NOT_LOADED);
}

inline void Property::changeContainer(Properties* container, const MEtl::string& var, const MEtl::string& val) {
  _container = container;
  _container->add(this);
  _container->setProperty(var, val, Property::NOT_LOADED);
}

inline void Property::changeContainer(Properties* container) {
  _container = container;
}

// This class contains the items that are to be relevant for all Properties classes.
class MetaProperties : public Properties {
public:
  MetaProperties(const char* sectionName);
  RWProperT<MEtl::string> unknownFieldPolicy;
  RWProperT<unsigned int> verbosity;
  RWProperT<MEtl::string> type;
  RWProperT<MEtl::string> objectsWithSameSectionNamePolicy;
};

/**
 * @brief Removes leading '/' from the prefix of a file name for offline and embedded Linux systems platform.
 *
 * @param fileName The file name to be modified.
 * @param isOffline A boolean indicating if the it is an offline run.
 *
 * @return A pointer to a modified file name if certain conditions are met.
 *         Otherwise, it returns the original file name.
 */
extern const char* Properties_FixPrefixFileName(const char* fileName, bool isOffline);

/**
 * @brief Reads data from a file into a string.
 *
 * Reads the contents of the specified file into the provided string.
 * The function opens the file in binary mode, reads the input stream,
 * and checks if the content is a printable string.
 *
 * @param[out] data Reference to a string where the file data will be stored.
 *                  Updated by this function to contain the read data.
 * @param[in] filename The name of the file to be read.
 *                     Should be a valid file path.
 *
 * @return True if the file is successfully read and the content is a printable string.
 *         False if the file cannot be read or the content is not a printable string.
 */
extern bool Properties_ReadFile(MEtl::string& data, const char* filename);

/**
 * @brief Reads input stream data into a string.
 *
 * This function reads data from the specified input stream into the provided string.
 * It iterates through the input stream character by character until the end of the stream,
 * appending each character to the string. After reading, it clears the input stream's error flags.
 *
 * @param[out] streamStr Reference to a string where the input stream data will be stored.
 *                       Updated by this function to contain the read data.
 * @param[in] inStream Reference to an input stream from which data will be read.
 */
extern void ReadInputStream(MEtl::string& streamStr, std::istream& inStream);

/**
 * @brief Reads and concatenates input stream data into a string.
 *
 * This function reads data from the specified input stream into the provided string.
 * It iterates through the input stream character by character until the end of the stream,
 * appending each character to the string. After reading, it clears the input stream's error flags.
 *
 * @param[out] streamStr Reference to a string where the input stream data will be concatenated.
 *                       Updated by this function to contain the concatenated data.
 * @param[in] inStream Reference to an input stream from which data will be read.
 *
 * @return True if the operation successfully reads and concatenates the input stream data.
 *         False otherwise, indicating a failure or end of input stream.
 */
extern bool ReadAndConcatenateInputStream(MEtl::string& streamStr, std::istream& inStream);

/**
 * @brief Checks if a given string contains printable characters only.
 *
 * This function examines the provided string to determine if it comprises
 * only printable characters or white spaces. If the verbosity level (g_verbose)
 * is greater than 1, it prints diagnostic information about the string length
 * and content to stdout.
 *
 * @param str The string to be checked for printable characters.
 *
 * @return True if the string contains only printable characters or white spaces.
 *         False if the string contains any non-printable characters.
 */
extern bool IsPrintString(const MEtl::string& str);

// TODO: delete this function
extern void Properties_HandleBadSectionsFromCmdLine();

/**
 * @brief Checks that the destination properties do not contain properties with the same name as source properties.
 *
 * @param src Reference to the source Properties object.
 * @param dst Reference to the destination Properties object.
 * @param verbose An integer representing the verbosity level for error reporting.
 *                Set to 0 to suppress error messages, or >0 to enable error messages.
 *
 * @return True if the destination Properties object do not contain properties with the same names as the source
 *         properties, otherwise returns false.
 */
extern bool Properties_checkPropertiesFields(const Properties& src, const Properties& dst, int verbose = 0);

/**
 * @brief Retrieves a list of sections from an INI-formatted string.
 *
 * This function extracts a list of sections enclosed in square brackets ([section_name])
 * from the provided INI-formatted string `iniStr`. It clears the provided `sections` list
 * before populating it with the extracted section names.
 *
 * @param[out] sections Reference to a list where the extracted section names will be stored.
 * @param[in] iniStr The INI-formatted string containing sections to be extracted.
 *
 * @return True if the function successfully extracts and populates the list with sections.
 *         False if there is an error during extraction or if the provided string is empty.
 */
extern bool Properties_GetListOfSections(std::list<MEtl::string>& sections, const MEtl::string& iniStr);

/**
 * @brief Checks for the existence of a section within an INI-formatted string.
 *
 * This function determines whether a provided section exists within the
 * INI-formatted string `iniStr`. It first checks if the input string is empty,
 * and if so, returns false indicating the absence of a section.
 *
 * @param section The section name to be checked for existence within the string.
 * @param iniStr The INI-formatted string containing sections to be checked.
 *
 * @return True if the specified section exists within the provided INI-formatted string.
 *         False if the section is not found or if the input string is empty.
 */
extern bool Properties_isSectionExist(const MEtl::string& section, const MEtl::string& iniStr);

/**
 * @brief Retrieves a map of sections from an INI-formatted string.
 *
 * This function extracts sections along with their contents from the provided
 * INI-formatted string `iniStr` and stores them in a map (`sections`).
 * For each section found, it updates the map (`sections`) with the section name as
 * the key and the content as the value.
 *
 * Before populating the provided map with sections parsed from the provided string,
 * the function clears the map.
 *
 * @param[out] sections Reference to a map where sections and their contents will be stored.
 * @param[in] iniStr The INI-formatted string containing sections and their content to be extracted.
 *
 * @return True if the function successfully extracts and populates the map with sections and content.
 *         False if an error occurs during extraction.
 *
 * @note If the function returns 'false', consider the content of the `data` string unreliable or incomplete.
 */
extern bool Properties_GetMapOfSections(std::map<MEtl::string, MEtl::string>& sections, const MEtl::string& iniStr);

/**
 * @brief Retrieves data belonging to a specified section from an INI-formatted string.
 *
 * This function aims to extract data associated with a specific `section` from the provided
 * INI-formatted string `iniStr` and stores it in the `data` string reference.
 *
 * It iterates through the string starting from the beginning (`startPos`) to identify the target
 * section. Once the desired section is found, it determines its content and stores it in the `data`.
 *
 * @param[in] section The section name whose data needs to be retrieved.
 * @param[in] iniStr The INI-formatted string containing sections and their content.
 * @param[out] data Reference to a string where the extracted section data will be stored.
 *
 * @return True if the function successfully extracts and stores the section data in `data`.
 *         False if the section is not found or an error occurs during extraction.
 *
 * @note If the function returns 'false', consider the content of the `data` string unreliable or incomplete.
 */
extern bool Properties_GetSectionData(const char* section, const MEtl::string& iniStr, MEtl::string& data);

/**
 * @brief Converts command line arguments to a calib format string.
 *
 * This function converts the given command line arguments 'argv' (with 'argc' count) into
 * a calib format string 'cmdCalibStr'. It leverages the 'Properties_ConvertCommandLineToFileFormat'
 * function by using a colon (':') as a separator between key-value pairs.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Array of command line arguments.
 * @param[out] cmdCalibStr Reference to store the converted calib format string.
 *
 * @return True if the conversion is successful; otherwise, false.
 */
extern bool Properties_ConvertCommandLineToCalibFormat(int argc, char* argv[], MEtl::string& cmdCalibStr);

/**
 * @brief Retrieves a section from an INI-formatted string.
 *
 * This function extracts a section from an INI-formatted string provided as `iniStr`.
 * It starts the extraction from the position `pos` within the string.
 * If `pos` is greater than the length of the input string (`iniStr`),
 * it indicates an error and returns false.
 *
 * The function searches for a section enclosed in square brackets ([section_name])
 * and returns the extracted section content in the `section` string.
 * If a section is found, the function updates the `pos` parameter to the index
 * of the character following the identified section's ending bracket.
 *
 * @param[in] iniStr The INI-formatted string containing section information.
 * @param[out] section Reference to a string where the extracted section will be stored.
 * @param[in] pos Reference to a size_t variable indicating the starting position for section extraction.
 *
 * @return True if a section is successfully found and extracted from the string.
 *         False if the provided position exceeds the string length or an error occurs during extraction.
 */
extern bool Properties_GetSection(const MEtl::string& iniStr, MEtl::string& section, size_t& pos);

/**
 * @brief Converts command line arguments to an INI format string.
 *
 * This function converts the given command line arguments 'argv' (with 'argc' count) into
 * an INI format string 'iniStr'. It utilizes the 'Properties_ConvertCommandLineToFileFormat'
 * function by using an equal sign ('=') as a separator between key-value pairs.
 *
 * @param argc Number of command line arguments.
 * @param argv Array of command line arguments.
 * @param iniStr Reference to store the converted INI format string.
 *
 * @return True if the conversion is successful; otherwise, false.
 */
extern bool Properties_ConvertCommandLineToIniFormat(int argc, char* argv[], MEtl::string& iniStr);

/**
 * @brief Converts an INI formatted string to a command line representation.
 *
 * This function takes an INI formatted string and converts it into a command line
 * representation. It processes sections and their properties from the INI string
 * and constructs a command line string using the information obtained.
 *
 * @param[out] commandLine Reference to a string where the generated command line will be stored.
 * @param[in] iniStr The INI formatted string to be converted.
 *
 * @return Returns true if the conversion succeeds, false otherwise.
 */
extern bool Properties_ConvertIniFormatToCommandLine(MEtl::string& commandLine, const MEtl::string& iniStr);

/**
 * @brief Parses a command line string and allocates memory for arguments.
 *
 * This function parses the provided command line string and allocates memory
 * for storing individual arguments, updating the argc and argv pointers to
 * facilitate executing commands using execv.
 *
 * @param[out] argc Pointer to an integer storing the number of arguments.
 *                  Updated by this function to reflect the number of parsed arguments.
 * @param[out] argv Pointer to a pointer to a character array storing arguments.
 *                  Updated by this function to contain allocated memory for arguments.
 * @param[in] cmdLine The command line string to be parsed and processed.
 *                    Should not be NULL.
 */
extern void Properties_AllocateCommandLine(int* argc, char** argv[], const char* cmdLine);

/**
 * @brief Releases memory allocated for command line arguments.
 *
 * This function is responsible for freeing the memory allocated for
 * command line arguments previously allocated by Properties_AllocateCommandLine.
 *
 * @param argc The number of arguments previously allocated.
 * @param argv Pointer to an array of strings containing command line arguments.
 *             This pointer and its content will be deallocated.
 */
extern void Properties_ReleaseCommandLine(int argc, char* argv[]);

/**
 * @brief Initializes command line global parameters.
 *
 * This global parameters are then used  by the function Properties_GetSettings.
 * Use Properties_Term() in order to reset these global parameters.
 *
 * @param argc The number of command line arguments.
 * @param argv The array of command line arguments.
 */
extern void Properties_Init(int argc, char* argv[]);

/**
 * @brief Resets command line  global parameters.
 *
 * This function performs the termination of the Properties system by resetting
 * global variables related to command line arguments (g_argc and g_argv).
 *
 * If the verbosity flag (g_verbose) is set, it prints a termination message to stderr.
 */
extern void Properties_Term();

/**
 * @brief Gets settings based on specified prefix and mode.
 *
 * This function retrieves settings based on the given prefix and mode. It expects
 * 'g_argc' and 'g_argv' to be initialized using the 'Properties_Init' function prior
 *  to calling this function.
 *
 * @param prefix The prefix for section matching.
 * @param mode The mode specifying the settings retrieval type:
 *             'P' - multi-line multi-sections from command line args
 *             'n' - multi-line single section from command line args
 *             's' - single line single section from command line args.
 *
 * @return A string containing the settings based on the specified prefix and mode.
 *         If 'mode' is 'P', this returns settings in INI format.
 *         For other modes ('n' and 's'), the function returns formatted settings.
 *         In case of failure or incorrect 'mode', an empty string is returned.
 *
 * @warning If g_argc and g_argv are not initialized when calling Properties_GetSettings,
 * the program will abort.
 */
extern std::string Properties_GetSettings(std::string prefix, char mode = '\0');

typedef std::map<MEtl::string, MEtl::string> SectionsMap;

#endif//__PROPERTIES__H__
