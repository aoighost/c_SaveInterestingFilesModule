Save Interesting Files Module
Sleuth Kit Framework C++ Module
May 2012


This module is for the C++ Sleuth Kit Framework.


DESCRIPTION

This module is a reporting module that saves files and directories
that were flagged as being interesting by the InterestingFiles module. 
It is used to extract the suspicious files for further anlaysis.
For example, you could use InterestingFiles to flag all files of
a given type and then use this module to save them to a local
folder for manual analysis. 


USAGE

Add this module to a post-processing/reporting pipeline.  See the TSK 
Framework documents for information on adding the module 
to the pipeline:

    http://www.sleuthkit.org/sleuthkit/docs/framework-docs/

The module takes the path to a folder where the files should be saved.


RESULTS

Intersting files are saved to the output folder in subdirectories bearing
the name given to the matching interesting files set in the configuration
file for the Interesting Files module.  File names are prefixed with
their file ids to avoid name collisions. The resulting directory structure
will night something like this:

        c:\img503\out\Interesting Files\
            ReadmeFiles\
                1_README
                24_readme.txt
                382_readme.txt

The contents of interesting directories are saved to the output folder in 
subdirectories bearing the name given to the matching interesting files set 
in the configuration file for the Interesting Files module.  A subdirectory 
is created with the same name as the directory, but prefixed with the
directory's file id to avoid name collisions. The contents of the directory,
including both files and subdirectories, is then saved. The resulting 
directory structure might look something like this:

        c:\img503\out\Interesting Files\
            SuspiciousDirs\
                1_bomb\
                    bomb\
                        intructions.txt
                        names.doc
                42_bomb\
                    bomb\
                        readme.txt
                        instructions\
                            intructions.txt
                            names.doc



