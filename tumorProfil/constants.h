#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * Used to save settings
 */

#define ORGANIZATION "TumorClinic"
#define APPLICATION "TumorProfil"


/**
 * Encryption constants
 */

#define USERID_INDEX         0
#define NAME_INDEX           1
#define USERGROUP_INDEX      2
#define PASSWORD_SALT_INDEX  3
#define PASSWORD_HASH_INDEX  4
#define AESFILLING_INDEX     5

#define KEY_NAME_INDEX       1
#define KEY_CONTENT_INDEX    3


/**
 * SQl table headers
 */

#define SQL_PATIENT_NAME "firstName"
#define SQL_PATIENT_SURNAME "surname"
#define SQL_PATIENT_DATEOFBIRTH "dataOfBirth"

#endif // CONSTANTS_H

