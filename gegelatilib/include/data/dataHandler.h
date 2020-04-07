#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <typeinfo>
#include <vector>
#include <functional>
#include <memory>

#include "data/untypedSharedPtr.h"

namespace std {
	/*
	* \brief  Equality operator to std to enable use of standard algorithm on vectors of reference_wrapper of const std::type_info.
	*/
	bool operator==(const std::reference_wrapper<const std::type_info>& r0, const std::reference_wrapper<const std::type_info>& r1);
}

namespace Data {
	/**
	* \brief Base class for all sources of data to be accessed by a TPG Instruction executed within a Program.
	*/
	class DataHandler {

	protected:

		/**
		* \brief Static count used to initialize the id of each DataHandler.
		*/
		static size_t count;

		/**
		* \brief Identifier of each DataHandler.
		*
		* This identifier should be used as a seed for the initialization
		* of the hash calculation.
		* Two DataHandler resulting from a copy should thus have the same id.
		*/
		const size_t id;

		/**
		* \brief Cached value returned by the getHash() function.
		*
		* The value of the hash is updated whenever the updateHash function is called.
		*/
		mutable size_t cachedHash;

		/**
		* \brief Boolean value indicating whether the current cachedValue is
		* valid, or not.
		*
		* When getting the value of the hash, it shall be automatically updated
		* when invalidCachedHash is set to true. Whenever the data contained in
		* a DataHandler is modifier, the invalidCachedHash attribute shall be
		* set to true.
		*
		*/
		mutable bool invalidCachedHash;

		/**
		* \brief Update the cachedHash value.
		*
		* This methods trigger an update of the cachedHash value and
		* returns the new value.
		*
		* \return the new value of the cachedhash attribute.
		*/
		virtual size_t updateHash() const = 0;

	public:
		/**
		* \brief Default constructor of the DataHandler class.
		*/
		DataHandler();

		/// Default destructor
		virtual ~DataHandler() = default;

		/**
		* \brief Default copy constructor.
		*/
		DataHandler(const DataHandler& other) = default;

		/**
		* \brief Return a copy of the DataHandler (with all its content).
		*
		* The returned copy shuold always have the same polymorphic type
		* as the original object, and give the same hash and data until
		* the original or the copy is modified.
		*
		* \return a pointer to the clone.
		*/
		virtual DataHandler* clone() const = 0;

		/**
		* \brief Get the ID of the DataHandler.
		*
		* Two DataHandler should have the same ID only if the are copy
		* from each other, possibly holding different data.
		* This property will be used to simplify check that two different
		* DataHandler have the exact same characteristics (handled types,
		* addressSpace, ..)
		*/
		size_t getId() const;

		/**
		* \brief Get the current value of the hash for this DataHandler.
		*
		* This method returns the value of the hash, and updates it if
		* necessary.
		*
		* \return the cached value of the Hash.
		*/
		size_t getHash() const;

		/**
		* \brief Check a given DataHandler can handle data for the given data type.
		*
		* \param[in] type the std::type_info whose availability in the DataHandler is being tested.
		* \return true if the DataHandler can handle data for the given data type, and false otherwise.
		*/
		virtual bool canHandle(const std::type_info& type) const = 0;


		/**
		* \brief Get the getAddressSpace size for the given data type.
		*
		* Since a single DataHandler may be able to provide data of different types, the addressable space may vary depending
		* on the accessed data type. This method returns the size of addressable data for each type of data.
		*
		* \param[in] type the std::type_info of data whose address space is retrieved.
		* \return the size of the retrieved address space, or 0 if the data type is not handled by the DataHandler.
		*/
		virtual size_t getAddressSpace(const std::type_info& type) const = 0;

		/**
		* \brief Get the largest AddressSpace for all data types handled by the DataHandler.
		*
		* This method relies on the getAddressSpace and getHandledTypes methods
		* to compute the size of the largest addressSpace required by the dataHandler.
		* \return the size of the largest addressSpace.
		*/
		virtual size_t getLargestAddressSpace() const = 0;

		/**
		* \brief Generic method for DataHandler to reset their data.
		*
		* Method used to reset the data handled by a DataHandler. Each
		* DataHandler can implement a custom behavior, or even no behavior at
		* all for this method.
		*
		* This method shall invalidate the cachedHash.
		*
		*/
		virtual void resetData() = 0;

		/**
		* \brief Get data of the given type, from the given address.
		*
		* Data is returned as an UntypedSharedPtr, with two possible allocations:
		* - Classic pointer: The returned data is natively contained in the
		* DataHandler and could be accessed through a regular pointer. In this
		* case the returned UntypedSharedPtr is associated with an empty destructor
		* function as its destructor to avoid any deallocation on the
		* shared_ptr deletion.
		* - Shared pointer: The returned data is a temporary object that was
		* constructed on request from data in the DataHandler. Once it has
		* been used, on deletion of the shared pointer, this temporary object
		* is deallocated using its default destructor.
		*
		* \param[in] type the std::type_info of data retrieved.
		* \param[in] address the location of the data to retrieve.
		* \throws std::invalid_argument if the given data type is not provided
		* by the DataHandler.
		* \throws std::out_of_range if the given address is invalid for the
		* given data type.
		* \return a shared pointer to the requested const data.
		*/
		virtual UntypedSharedPtr getDataAt(const std::type_info& type, const size_t address) const = 0;

		/**
		* \brief Get the set of addresses actually used when getting the given
		* type of data, at the given address.
		*
		*  When accessing a DataHandler with a type differing from the native
		* storage type of the DataHandler, like an array for example, the
		* DataHandler may need to use several of its data element to create
		* and return the requested type. Keeping track of what addresses have
		* been accessed and used may be usefull to better explain what part of
		* the data was used in a learning process, or to identify introns when
		* the DataHandler in question are the registers of the execution
		* engine. This method returns the list of addresses that are used when
		* requesting a given type of data, at a given address.
		*
		* \param[in] type the std::type_info of data whose access pattern is
		* analyzed.
		* \param[in] address the location of the data to retrieve.
		* \return a std::vector containing the addresses of data accessed. In
		* case the given type of data is invalid, or the address, an empty
		* vector is returned.
		*/
		virtual std::vector<size_t> getAddressesAccessed(const std::type_info& type, const size_t address) const = 0;
	};
}

#endif