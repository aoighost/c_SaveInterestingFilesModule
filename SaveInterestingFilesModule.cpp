/*
 * The Sleuth Kit
 *
 * Contact: Brian Carrier [carrier <at> sleuthkit [dot] org]
 * Copyright (c) 2010-2012 Basis Technology Corporation. All Rights
 * reserved.
 *
 * This software is distributed under the Common Public License 1.0
 */

/** \file InterestingFiles.cpp
 * This file contains the implementation of a module that saves interesting 
 * files recorded on the blackboard to a user-specified output directory.
 */

// System includes
#include <string>
#include <sstream>
#include <vector>

// Framework includes
#include "TskModuleDev.h"

// Poco includes
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/Exception.h"

namespace
{
    // The interesting files will be saved to this location. The path is passed to
    // the module as an argument to the initialize() module API and cached here 
    // for use in the report() module API.
    std::string outputFolderPath;

    // Helper function to create a directory. Throws TskException on failure.
    void createDirectory(const std::string &path)
    {
        try
        {
            Poco::File dir(path);
            dir.createDirectories();
        } 
        catch (Poco::Exception &ex) 
        {
            std::stringstream msg;
            msg << L"SaveInterestingFilesModule failed to create directory '" << outputFolderPath << "' : " << ex.message();
            throw TskException(msg.str());
        }
    }

    // Helper function to recursively write out the contents of a directory. Throws TskException on failure.
    void saveDirectoryContents(const std::wstring &dirPath, uint64_t dirFileId)
    {
        // Construct a query for the file records corresponding to the files in the directory and fetch them.
        std::stringstream condition; 
        condition << "WHERE par_file_id = " << dirFileId;
        std::vector<const TskFileRecord> files = TskServices::Instance().getImgDB().getFileRecords(condition.str());

        // Save each file and subdirectory in the directory.
        for (std::vector<const TskFileRecord>::const_iterator file = files.begin(); file != files.end(); ++file)
        {
            if ((*file).metaType == TSK_FS_META_TYPE_DIR)
            {
                // Create a subdirectory to hold the contents of this subdirectory.
                std::wstringstream subDirPath;
                subDirPath << dirPath << Poco::Path::separator() << (*file).name.c_str();
                createDirectory(TskUtilities::toUTF8(subDirPath.str()));

                // Recurse into the subdirectory.
                saveDirectoryContents(subDirPath.str(), (*file).fileId);
            }
            else
            {
                // Save the file.
                std::wstringstream filePath;
                filePath << dirPath << Poco::Path::separator() << (*file).name.c_str();
                TskServices::Instance().getFileManager().copyFile((*file).fileId, filePath.str());
            }
        }
    }

    // Helper function to save the contents of an interesting directory to the output folder. Throws TskException on failure.
    void saveInterestingDirectory(const TskFileRecord &dir, const std::string &setName)
    {
        // Make a subdirectory of the output folder named for the interesting file search set and create a further subdirectory
        // corresponding to the directory to be saved. The resulting directory structure will look like this:
        // <output folder>/
        //      <interesting file set name>/
        //          <file id>_<directory name>/ /*Prefix the directory with its its file id to ensure uniqueness*/
        //              <directory name>/
        //                  <contents of directory including subdirectories>
        //
        std::wstringstream path;
        path << outputFolderPath.c_str() << Poco::Path::separator() 
             << setName.c_str() << Poco::Path::separator() 
             << dir.fileId << L"_" << dir.name.c_str() << Poco::Path::separator() 
             << dir.name.c_str();
        createDirectory(TskUtilities::toUTF8(path.str()));

        saveDirectoryContents(path.str(), dir.fileId);

        // Log the result.
        std::wstringstream msg;
        msg << L"SaveInterestingFilesModule saved directory to '" << path.str() << L"'";
        LOGINFO(msg.str());
    }

    // Helper function to save the contents of an interesting file to the output folder. Throws TskException on failure.
    void saveInterestingFile(const TskFileRecord &file, const std::string &setName)
    {
        // Construct a path to write the contents of the file to a subdirectory of the output folder named for the interesting file search
        // set. The resulting directory structure will look like this:
        // <output folder>/
        //      <interesting file set name>/
        //          <file id>_<file name> /*Prefix the file with its its file id to ensure uniqueness*/
        std::wstringstream path;
        path << outputFolderPath.c_str() << Poco::Path::separator() 
             << setName.c_str() << Poco::Path::separator() 
             << file.fileId << L"_" << file.name.c_str();
        TskServices::Instance().getFileManager().copyFile(file.fileId, path.str());

        // Log the result.
        std::wstringstream msg;
        msg << L"SaveInterestingFilesModule saved file to '" << path.str().c_str() << L"'";
        LOGINFO(msg.str());
    }
}

extern "C" 
{
    /**
     * Module identification function. 
     *
     * @return The name of the module.
     */
    TSK_MODULE_EXPORT const char *name()
    {
        return "SaveInterestingFiles";
    }

    /**
     * Module identification function. 
     *
     * @return A description of the module.
     */
    TSK_MODULE_EXPORT const char *description()
    {
        return "";
    }

    /**
     * Module identification function. 
     *
     * @return The version of the module.
     */
    TSK_MODULE_EXPORT const char *version()
    {
        return "0.0.0";
    }

    /**
     * Module initialization function. Receives an output folder path as the location
     * for saving the files corresponding to interesting file set hits.
     *
     * @param args Output folder path.
     * @return TskModule::OK 
     */
    TSK_MODULE_EXPORT TskModule::Status initialize(const char* arguments)
    {
        // Reset the output folder path in case initialize() is called more than once.
        outputFolderPath.clear();

        std::wstringstream msg;
        if (arguments != NULL)
        {
            outputFolderPath = arguments;
            if (!outputFolderPath.empty()) 
            {
                msg << L"SaveInterestingFilesModule initialized with output folder path " << outputFolderPath.c_str();
                LOGINFO(msg.str());

            }
            else
            {
                msg << L"SaveInterestingFilesModule received empty output directory argument";
                LOGERROR(msg.str());
            }
        }
        else
        {
            msg << L"SaveInterestingFilesModule received NULL output directory argument";
            LOGERROR(msg.str());
        }

        // Always return OK when initializing a reporting/post-processing pipeline module 
        // so the pipeline is not disabled by the presence of a non-functional module.
        return TskModule::OK;
    }

    /**
     * Module execution function. saves interesting files recorded on the 
     * blackboard to a user-specified output directory.
     *
     * @returns TskModule::OK on success if all files saved, TskModule::FAIL if one or more files were not saved
     */
    TSK_MODULE_EXPORT TskModule::Status report()
    {
        TskModule::Status returnCode = TskModule::OK;
        
        LOGINFO(L"SaveInterestingFilesModule save operations started");

        try
        {
            // Make the output directory specified using the initialize() API.
            createDirectory(outputFolderPath);
            
            // Get the interesting file hit artifacts from the blackboard and save the corresponding files to the output directory.
            std::vector<TskBlackboardArtifact> files = TskServices::Instance().getBlackboard().getArtifacts(TSK_INTERESTING_FILE_HIT);
            for (std::vector<TskBlackboardArtifact>::iterator artifact = files.begin(); artifact != files.end(); ++artifact)
            {
                try
                {
                    // Get the file record corresponding to the hit.
                    TskFileRecord file;
                    if (TskServices::Instance().getImgDB().getFileRecord((*artifact).getObjectID(), file) == -1)
                    {
                        std::stringstream msg;
                        msg << "SaveInterestingFilesModule failed to get file record for file Id = " << (*artifact).getObjectID() << ", cannot save file";
                        throw TskException(msg.str());
                    }
                    
                    // Get the set name attibute from the artifact and save the file corresponding to the hit to a subdirectory of the output folder
                    // bearing the name of the interesting files set.
                    std::vector<TskBlackboardAttribute> attrs = (*artifact).getAttributes();
                    for (std::vector<TskBlackboardAttribute>::iterator attr = attrs.begin(); attr != attrs.end(); ++attr)
                    {
                        if ((*attr).getAttributeTypeID() == TSK_SET_NAME)
                        {
                            if (file.metaType == TSK_FS_META_TYPE_DIR)
                            {
                                 saveInterestingDirectory(file, (*attr).getValueString()); 
                            }
                            else
                            {
                                saveInterestingFile(file, (*attr).getValueString());
                            }
                        }
                    }
                }
                catch(TskException &ex)
                {
                    // Log the error and try the next file hit, but signal that an error occurred with a FAIL return code.
                    LOGERROR(TskUtilities::toUTF16(ex.message()));
                    returnCode = TskModule::FAIL;
                }
            }
        }
        catch(TskException &ex)
        {
            LOGERROR(TskUtilities::toUTF16(ex.message()));
            returnCode = TskModule::FAIL;
        }

        LOGINFO(L"SaveInterestingFilesModule save operations finished");
        
        return returnCode;
    }

    /**
     * Module cleanup function. This imodule does not need to free any resources 
     * allocated during initialization or execution.
     *
     * @returns TskModule::OK
     */
    TSK_MODULE_EXPORT TskModule::Status finalize()
    {
        return TskModule::OK;
    }
}
