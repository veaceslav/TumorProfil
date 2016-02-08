##Useful information:

Login:
mysql -u root -p

Create database:
CREATE DATABASE blog;

grant permission:
GRANT ALL ON wpblog.* TO wpuser@localhost IDENTIFIED BY 'foobar';

mysql -u wpuser -p wpblog

create user:

CREATE USER 'jeffrey'@'localhost' IDENTIFIED BY 'mypass';

##Troubleshooting:

Error when TumorProfil creates tables:

If you get this error in console:
Error messages: "QMYSQL: Unable to execute query" "You do not have the SUPER privilege and binary logging is enabled (you *might* want to use the less safe log_bin_trust_function_creators variable)" 1419 2 
Bound values:  ()
Error while executing DBAction [ "CreateDBTrigger" ] Statement [ "\n                CREATE TRIGGER delete_patient AFTER DELETE ON Patients\n                FOR EACH ROW\n                BEGIN\n                    DELETE FROM Diseases          WHERE patientid=OLD.id;\n                    DELETE FROM PatientProperties WHERE patientid=OLD.id;\n                END;\n                " ]

you must do this:

mysql -u USERNAME -p

set global log_bin_trust_function_creators=1;