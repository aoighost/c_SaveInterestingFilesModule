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
 * This file contains the implementation of the save interesting files module.
 * The module saves interesting files recorded on the blackboard to
 * a user-specified output directory.
 */

// System includes
#include <sstream>
#include <fstream>
#include <map>

// Framework includes
#include "TskModuleDev.h"

// Poco includes
#include "Poco/FileStream.h"
#include "Poco/AutoPtr.h"
#include "Poco/UnicodeConverter.h"
#include "Poco/Path.h"
#include "Poco/File.h"

static std::string outputDir;

extern "C" 
{
    /**
     * Module initialization function. Receives a string of intialization arguments, 
     * typically read by the caller from a pipeline configuration file. 
     * Returns TskModule::OK or TskModule::FAIL. Returning TskModule::FAIL indicates 
     * the module is not in an operational state.  
     *
     * @param args Output directory path.
     * @return TskModule::OK if initialization succeeded, otherwise TskModule::FAIL.
     */
    TskModule::Status TSK_MODULE_EXPORT initialize(std::string& args)
    {
        if (args.empty()) 
        {
            std::wstringstream msg;
            msg << L"  SaveInterestingFiles Module: Missing output directory argument.";
            LOGERROR(msg.str());
            return TskModule::FAIL;
        }
        else
        {
            std::wstringstream msg;
            outputDir = args;
            // strip off quotes if they were passed in via XML
            if (outputDir[0] == '"')
                outputDir.erase(0, 1);
            if (outputDir[outputDir.size()-1] == '"')
                outputDir.erase(outputDir.size()-1, 1);

            try {
                Poco::File dirFile(outputDir);
                dirFile.createDirectories();
                Poco::Path path(outputDir);
                if (!(dirFile.isDirectory() && dirFile.canWrite())) {
                    std::wstringstream msg;
                    msg << L"  SaveInterestingFiles Module: Failed to create directory: " << outputDir.c_str();
                    LOGERROR(msg.str());
                    return TskModule::FAIL;
                } else {
                    msg << L"  SaveInterestingFiles Module: Initialized with argument: " << outputDir.c_str();
                    LOGINFO(msg.str());
                }
            } catch (std::exception & ex) {
                std::wstringstream msg;
                msg << L"  SaveInterestingFiles Module: Failed to create directory: " << outputDir.c_str() << " Exception: " << ex.what();
                LOGERROR(msg.str());
                return TskModule::FAIL;
            }
        }
        return TskModule::OK;
    }

    /**
     * Module execution function. Returns TskModule::OK, TskModule::FAIL, or TskModule::STOP. 
     * Returning TskModule::FAIL indicates error performing its job. Returning TskModule::STOP
     * is a request to terminate execution of the reporting pipeline.
     *
     * @returns TskModule::OK on success, TskModule::FAIL on error, or TskModule::STOP.
     */
    TskModule::Status TSK_MODULE_EXPORT report()
    {
        TskModule::Status result = TskModule::OK;

        if (outputDir == "") 
        {
            LOGERROR(L"  SaveInterestingFiles Module: OutputDir is empty.");
            return TskModule::FAIL;
        }

        LOGINFO(L"  SaveInterestingFiles Module: run");

        TskImgDB & imgdb = TskServices::Instance().getImgDB();
        TskFileManager& fileManager = TskServices::Instance().getFileManager();
        TskBlackboard & blackboard = TskServices::Instance().getBlackboard();
        std::vector<TskBlackboardAttribute> attributes = blackboard.getAttributes(TSK_INTERESTING_FILE);

        std::wstringstream msg;
        msg << "  SaveInterestingFiles Module: Found " << attributes.size() << " interesting files.";
        LOGINFO(msg.str());

        for (size_t i = 0; i < attributes.size(); i++) {
            try {
                uint64_t fileId = attributes[i].getParentArtifact().getObjectID();
                TskFileRecord fileRec;
                if (imgdb.getFileRecord(fileId, fileRec) != -1) {
                    std::stringstream outputPath;
                    outputPath << outputDir << Poco::Path::separator() << fileRec.fileId << "_" << fileRec.name;
                    std::wstring wPath = TskUtilities::toUTF16(outputPath.str());
                    fileManager.copyFile(fileRec.fileId, wPath);
                    msg.str(L"");
                    msg << L"  SaveInterestingFiles Module: saved file: " << wPath;
                    LOGINFO(msg.str());
                } else {
                    msg.str(L"");
                    msg << L"  SaveInterestingFiles Module: getFileRecord failed for fileId = " << fileId;
                    LOGERROR(msg.str());
                }
            } catch (const std::exception& ex) {
                std::wstringstream msg;
                msg << L"  SaveInterestingFiles Module: exception: " << ex.what();
                LOGERROR(msg.str());
                result = TskModule::FAIL;
            }
        }
        return result;
    }

    /**
     * Module cleanup function. This is where the module should free any resources 
     * allocated during initialization or execution.
     *
     * @returns TskModule::OK on success and TskModule::FAIL on error.
     */
    TskModule::Status TSK_MODULE_EXPORT finalize()
    {
        return TskModule::OK;
    }
}
