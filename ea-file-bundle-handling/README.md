# ea-file-bundle-handling

This directory contains scripts to manage files on a linux file system in conjunction with their extended attributes.
Since extended attributes are native to any ext4, btrfs or openxfs file system, extended attributes are managed alonside their
owner files just fine by the file system.

These scripts originated from an earlier (now legacy) repository 'synology-scripts' where careful management of sidecar files was necessary, but also introduce some subtle behaviour around overwriting files, that the native mv, rm and ln commands don't have. So, in this new repository, most of these scripts aren't doing anything spectacular and I expect to phase them out over time.

The most useful xattr for me is the **`com.apple.metadata:_kMDItemUserTags`**, which is the xattr in which the user tags are stored (both the 'old-style' labels (Finder colours) and the custom Tags. See the directory with tools for handling extended attributes and Finder tags.
