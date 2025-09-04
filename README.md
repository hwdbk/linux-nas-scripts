# linux-nas-scripts

A collection of (primarily bash) scripts (and some C++ code) that I've developed over the years.
These scripts are generic to linux and should work on any distribution.

There are scripts for:
- **extended-attributes-and-tags**: retrieving and managing extended attributes that are set from MacOS (using samba), such as custom tags and labels and user xattrs, and access them natively from the linux file system.
- **ea-file-bundle-handling**: managing files with extended attributes without creating inconsistencies in the file system. This is a legacy remnant from the Synology age where the system uses sidecar files to store extended attributes. Maintained here (for a while) because it contains special logic around deleting files with (hard)link count >1 (ties into **linked-collections**).
- **linked-collections** generating and managing directories with hard linked files based on custom criteria, effectively creating collections or repositories from other (source) directories, such as music playlists or movie genres (well, that's what I use it for; your mileage may vary).
- **mac-nfd-conversion**: convert file name character representation between native/normal UTF-8 and MacOSX NFD (normalization form decomposed) representation.
- **file-tracker**: generating checksum files that allow you to keep track of your files, wherever they go.
- **multi-disk-backup**: create a backup spanning multiple disks (if necessary), where each disk contains (a managed subset of) the data in its backup folder. The script generates an index and log which allows you to use the disk(s) individually to restore data (as long as you remember the passphrase). Can be with or without encryption.

In order to use these, you'll need to be able to execute these scripts from the command line, which you can do from an ssh session or from a cron job.
