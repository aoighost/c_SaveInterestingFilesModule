Save Interesting Files Module
Sleuth Kit Framework C++ Module
May 2012


This module is for the C++ Sleuth Kit Framework.


DESCRIPTION

This module is a reporting module that saves files that were 
flagged as being interesting by the InterestingFiles module. 
It is used to extract the suspicious files for further anlaysis.
For example, you could use InterestingFiles to flag all files of
a given type and then use this module to save them to a local
folder for manual analysis. 


USAGE

Add this module to a post-processing/reporting pipeline.  See the TSK 
Framework documents for information on adding the module 
to the pipeline:

    http://www.sleuthkit.org/sleuthkit/docs/framework-docs/

The module takes the path to where the files should be saved.


RESULTS

The files are saved to the specified folder. 

Currently, they are saved in a single directory and renamed to
have their unique file ID (to prevent naming collisions).

The module will currently fail if a folder has been identified
as interesting.


TODO
- Expand to support directories.
