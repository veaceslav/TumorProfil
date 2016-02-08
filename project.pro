TEMPLATE = subdirs


tumorProfil.subdir = tumorProfil
tumorProfil.depends = cryptopp563

tumorUsers.subdir = tumorProfil/TumorUsers
tumorUsers.depends = cryptopp563

SUBDIRS = cryptopp563 \
          tumorProfil \
          tumorUsers
