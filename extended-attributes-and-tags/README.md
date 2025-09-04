# Extended attributes and tags (as seen from the linux side)

This directory contains a toolset to retrieve Mac OS X -created extended attributes natively from the linux native file system. For a collection
of excellent articles on what extended attributes are and how/what for they are used in Mac OS X, read
https://eclecticlight.co/2020/10/24/theres-more-to-files-than-data-extended-attributes/ and it's related click-through Related articles at the bottom.

I was triggered on this by new funcionality in the Finder that allowed a user not only to tag a file with Labels (the 'Finder colours' red, orange, yellow, green, blue, purple and gray), but also custom tags. Then I found out that these were stored in the Apple-proprietary `com.apple.metadata:_kMDItemUserTags` extended attribute and that jdberry wrote a cool tool to manage them from the (Mac OS X Terminal) command line:

https://github.com/jdberry/tag

The scripts/programs provided here attempt to do the same natively from the linux side of the samba (smb:// file share) connection. A C++ (.cpp) version is available and easy to compile - a compiled C++ program is much faster than a bash script. For a (no longer maintained) bash version see in repository synology-scripts/extended-attributes-and-tags `get_attr.sh` and `tag.sh`.

Extracting the tags works in two stages:
- the `get_attr` (compiled `get_attr.cpp` program) retrieves the extended attribute raw data from a file's native extended attributes. This can be used for any extended attribute, even your own. Output can be in hex or plain text.
- the `tag` (compiled `tag.cpp` program) calls `get_attr()` and interprets this raw data according to the Apple bplist data structures laid out for the `com.apple.metadata:_kMDItemUserTags` extended attribute. It prints the tags in a comma-separated list or on separate lines, with or without file name, just like jdberry's `tag` (see usage for (sup)ported options).
- the `listtags` script calls `tag` (implying `-lnG`) for all files (and directories, recursively) passed on the commandline. It prints the tags as comma-separated list on one line per file, just like jdberry's `tag -lR` would do, but skipping files that have no tags and printing the full path. Useful to keep a backup copy of all tags, or to check/track tags when making changes to the filesystem.

Finally, extended attributes (and hence, tags) are retrieved read-only. That is because I haven't yet ported the code to recreate the structure of the bplist from scratch and write it to the `com.apple.metadata:_kMDItemUserTags` extended attribute.
