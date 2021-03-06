#ifndef GEM_ONLINEDB_PARTREFERENCE_H
#define GEM_ONLINEDB_PARTREFERENCE_H

#include <string>

namespace gem {
    namespace onlinedb {

        /**
         * @brief References a part by its barcode.
         */
        struct PartReferenceBarcode
        {
            std::string barcode;

            /**
             * @brief Checks two barcode part references for equality.
             */
            bool operator== (const PartReferenceBarcode &other) const
            {
                return barcode == other.barcode;
            }
        };

        /**
         * @brief References a part by its serial number.
         */
        struct PartReferenceSN
        {
            std::string serialNumber;

            /**
             * @brief Checks two S/N part references for equality.
             */
            bool operator== (const PartReferenceSN &other) const
            {
                return serialNumber == other.serialNumber;
            }
        };

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_PARTREFERENCE_H
