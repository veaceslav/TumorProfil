TEMPLATE = subdirs


tumorProfil.subdir = tumorProfil

win32{
    tumorProfil.depends = cryptopp563
}

tumorUsers.subdir = tumorProfil/TumorUsers

win32{
    tumorUsers.depends = cryptopp563
}

SUBDIRS = tumorProfil \
          tumorUsers

win32 {
    SUBDIRS+= cryptopp563
}
