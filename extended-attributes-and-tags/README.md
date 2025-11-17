# Extended attributes and tags (as seen from the linux side)

This directory contains a toolset to retrieve Mac OS X -created extended attributes natively from the linux native file system. For a collection
of excellent articles on what extended attributes are and how/what for they are used in Mac OS X, read
https://eclecticlight.co/2020/10/24/theres-more-to-files-than-data-extended-attributes/ and it's related click-through Related articles at the bottom.

I was triggered on this by new funcionality in the Finder that allowed a user not only to tag a file with Labels (the 'Finder colours' red, orange, yellow, green, blue, purple and gray), but also custom tags. Then I found out that these were stored in the Apple-proprietary `com.apple.metadata:_kMDItemUserTags` extended attribute and that jdberry wrote a cool tool to manage them from the (Mac OS X Terminal) command line:

https://github.com/jdberry/tag

The scripts/programs provided here attempt to do the same natively from the linux side of the samba (smb:// file share) connection. A C++ (.cpp) version is available and easy to compile - a compiled C++ program is much faster than a bash script. For a (no longer maintained) bash version see in repository synology-scripts/extended-attributes-and-tags `get_attr.sh` and `tag.sh`.

There are many different ways that Mac OS Finder tags can be represented on a (linux) file server. I don't have to talk about afp as the sharing protocol any more because it is deprecated, not adviced any more and effectively replaced by Samba (smb). But even Samba has several options to map and store extended attributes passed over the smb:// connection. This is important as it intrinsically determines where/how the data is stored and whether the software here will work or not. The software assumes the use of the following options in the `[global]` or `[share]` section(s) in `/etc/samba/smb.conf`:
```
ea support = yes
vfs objects = catia fruit streams_xattr
fruit:aapl = yes
fruit:metadata = stream
fruit:resource = stream
fruit:encoding = native
```
This will instruct Samba to store the extended attributes from Mac OS passed over smb:// as a stream in the file system's native extended attributes (xattr) instead of in AppleDouble sidecar files (UGH), and specifically in the `user.DosStream.com.apple.metadata:_kMDItemUserTags:$DATA` extended attribute. Note that Samba (apparently) adds some namespace prefix and `$DATA` after the attribute name that Apple uses (`com.apple.metadata:_kMDItemUserTags`).

Synology note: note that Synology stores extended attributes fundamentally different - they use sidecar files in the @eaDir subdirectories and don't rely on extended attributes in the file system. See my synology-scripts github site to interpret and use these sidecar files.

UGreen note: although UGreen's UGOS is based on Debian 12 'bookworm', their Samba implementation is still somewhat custom, uses different settings than above and has a vfs module enabled called `ug_xattr_filter` which hints at some custom mapping going on in the background - and as a result, xattrs are not stored as per standard linux samba and hence the scripts/programs published here don't (won't) work.

Extracting the tags works in two stages:
- the `get_attr` (compiled `get_attr.cpp` program) retrieves the extended attribute raw data from a file's native extended attributes. This can be used for any extended attribute, even your own. Output can be in hex or plain text.
- the `tag` (compiled `tag.cpp` program) calls `get_attr()` and interprets this raw data according to the Apple bplist data structures laid out for the `com.apple.metadata:_kMDItemUserTags` extended attribute. It prints the tags in a comma-separated list or on separate lines, with or without file name, just like jdberry's `tag` (see usage for (sup)ported options).
- the `listtags` script calls `tag` (implying `-lnG`) for all files (and directories, recursively) passed on the commandline. It prints the tags as comma-separated list on one line per file, just like jdberry's `tag -lR` would do, but skipping files that have no tags and printing the full path. Useful to keep a backup copy of all tags, or to check/track tags when making changes to the filesystem.

Finally, extended attributes (and hence, tags) are retrieved read-only. That is because I haven't yet ported the code to recreate the structure of the bplist from scratch and write it to the `com.apple.metadata:_kMDItemUserTags` extended attribute.
