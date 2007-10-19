/*
 *
 *  Copyright (C) 1994-2007, OFFIS
 *
 *  This software and supporting documentation were developed by
 *
 *    Kuratorium OFFIS e.V.
 *    Healthcare Information and Communication Systems
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *  THIS SOFTWARE IS MADE AVAILABLE,  AS IS,  AND OFFIS MAKES NO  WARRANTY
 *  REGARDING  THE  SOFTWARE,  ITS  PERFORMANCE,  ITS  MERCHANTABILITY  OR
 *  FITNESS FOR ANY PARTICULAR USE, FREEDOM FROM ANY COMPUTER DISEASES  OR
 *  ITS CONFORMITY TO ANY SPECIFICATION. THE ENTIRE RISK AS TO QUALITY AND
 *  PERFORMANCE OF THE SOFTWARE IS WITH THE USER.
 *
 *  Module:  dcmnet
 *
 *  Author:  Marco Eichelberg / Andrew Hewett
 *
 *  Purpose: Classes for Query/Retrieve Service Class User (C-FIND operation)
 *
 *  Last Update:      $Author: onken $
 *  Update Date:      $Date: 2007-10-19 10:56:33 $
 *  Source File:      $Source: /export/gitmirror/dcmtk-git/../dcmtk-cvs/dcmtk/dcmnet/libsrc/dfindscu.cc,v $
 *  CVS/RCS Revision: $Revision: 1.3 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/dcmnet/dfindscu.h"

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#define INCLUDE_CSTDARG
#define INCLUDE_CERRNO
#include "dcmtk/ofstd/ofstdinc.h"

#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdicent.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/ofstd/ofconapp.h"

/* ---------------- static functions ---------------- */

#define OFFIS_CONSOLE_APPLICATION "findscu"

static void progressCallback(
  void *callbackData,
  T_DIMSE_C_FindRQ *request,
  int responseCount,
  T_DIMSE_C_FindRSP *rsp,
  DcmDataset *responseIdentifiers)
{
  DcmFindSCUDefaultCallback *callback = OFreinterpret_cast(DcmFindSCUDefaultCallback *, callbackData);
  if (callback) callback->callback(request, responseCount, rsp, responseIdentifiers);
}

/* ---------------- class DcmFindSCUCallback ---------------- */

DcmFindSCUCallback::DcmFindSCUCallback()
: assoc_(NULL)
, presId_(0)
{
}
  
void DcmFindSCUCallback::setAssociation(T_ASC_Association *assoc)
{
  assoc_ = assoc;
}

void DcmFindSCUCallback::setPresentationContextID(T_ASC_PresentationContextID presId)
{
  presId_ = presId;
}

/* ---------------- class DcmFindSCUCallback ---------------- */

DcmFindSCUDefaultCallback::DcmFindSCUDefaultCallback(
    OFBool extractResponsesToFile,
    int cancelAfterNResponses,
    OFBool verbose)	
: DcmFindSCUCallback()
, extractResponsesToFile_(extractResponsesToFile)
, cancelAfterNResponses_(cancelAfterNResponses)
, verbose_(verbose)
{
}

void DcmFindSCUDefaultCallback::callback(
        T_DIMSE_C_FindRQ *request,
        int responseCount,
        T_DIMSE_C_FindRSP *rsp,
        DcmDataset *responseIdentifiers)
 {
    /* dump response number */
    STD_NAMESPACE ostream& mycout = ofConsole.lockCout();
    mycout << "RESPONSE: " << responseCount << " (" << DU_cfindStatusString(rsp->DimseStatus) << ")\n";

    /* dump data set which was received */
    responseIdentifiers->print(mycout);
   
    /* dump delimiter */
    mycout << "--------" << OFendl;
    ofConsole.unlockCout();
    
    /* in case extractResponsesToFile is set the responses shall be extracted to a certain file */
    if (extractResponsesToFile_) {
        char rspIdsFileName[1024];
        sprintf(rspIdsFileName, "rsp%04d.dcm", responseCount);
        DcmFindSCU::writeToFile(rspIdsFileName, responseIdentifiers);
    }

    /* should we send a cancel back ?? */
    if (cancelAfterNResponses_ == responseCount)
    {
        if (verbose_)
        {
            ofConsole.lockCout() << "Sending Cancel RQ, MsgId: " << request->MessageID << ", PresId: " << presId_ << OFendl;
            ofConsole.unlockCout();
        }
        OFCondition cond = DIMSE_sendCancelRequest(assoc_, presId_, request->MessageID);
        if (cond.bad())
        {
            ofConsole.lockCerr() << "Cancel RQ Failed:" << OFendl;
            ofConsole.unlockCerr();
            DimseCondition::dump(cond);
        }
    }
}

/* ---------------- class DcmFindSCU ---------------- */


DcmFindSCU::DcmFindSCU(OFBool verboseMode, OFBool debugMode)
: net_(NULL), verbose_(verboseMode), debug_(debugMode)
{
}

DcmFindSCU::~DcmFindSCU()
{ 
  dropNetwork(); 
}



void DcmFindSCU::addOverrideKey(DcmDataset * & overrideKeys, OFConsoleApplication& app, const char* s)
{
    unsigned int g = 0xffff;
    unsigned int e = 0xffff;
    int n = 0;
    char val[1024];
    OFString dicName, valStr;
    OFString msg;
    char msg2[200];
    val[0] = '\0';

    // try to parse group and element number
    n = sscanf(s, "%x,%x=%s", &g, &e, val);
    OFString toParse = s;
    size_t eqPos = toParse.find('=');
    if (n < 2)  // if at least no tag could be parsed
    { 
      // if value is given, extract it (and extrect dictname)
      if (eqPos != OFString_npos)
      {
        dicName = toParse.substr(0,eqPos).c_str();
        valStr = toParse.substr(eqPos+1,toParse.length());
      }
      else // no value given, just dictionary name
        dicName = s; // only dictionary name given (without value)
      // try to lookup in dictionary
      DcmTagKey key(0xffff,0xffff);
      const DcmDataDictionary& globalDataDict = dcmDataDict.rdlock();
      const DcmDictEntry *dicent = globalDataDict.findEntry(dicName.c_str());
      dcmDataDict.unlock();
      if (dicent!=NULL) {
        // found dictionary name, copy group and element number
        key = dicent->getKey();
        g = key.getGroup();
        e = key.getElement();
      }
      else {
        // not found in dictionary
        msg = "bad key format or dictionary name not found in dictionary: ";
        msg += dicName;
        app.printError(msg.c_str());
      }
    } // tag could be parsed, copy value if it exists
    else
    {
      if (eqPos != OFString_npos)
        valStr = toParse.substr(eqPos+1,toParse.length());
    }
    DcmTag tag(g,e);
    if (tag.error() != EC_Normal) {
        sprintf(msg2, "unknown tag: (%04x,%04x)", g, e);
        app.printError(msg2);
    }
    DcmElement *elem = newDicomElement(tag);
    if (elem == NULL) {
        sprintf(msg2, "cannot create element for tag: (%04x,%04x)", g, e);
        app.printError(msg2);
    }
    if (valStr.length() > 0) {
        if (elem->putString(valStr.c_str()).bad())
        {
            sprintf(msg2, "cannot put tag value: (%04x,%04x)=\"", g, e);
            msg = msg2;
            msg += valStr;
            msg += "\"";
            app.printError(msg.c_str());
        }
    }

    if (overrideKeys == NULL) overrideKeys = new DcmDataset;
    if (overrideKeys->insert(elem, OFTrue).bad()) {
        sprintf(msg2, "cannot insert tag: (%04x,%04x)", g, e);
        app.printError(msg2);
    }
}


OFCondition DcmFindSCU::initializeNetwork(int acse_timeout)
{
  return ASC_initializeNetwork(NET_REQUESTOR, 0, acse_timeout, &net_);
}

OFCondition DcmFindSCU::setTransportLayer(DcmTransportLayer *tLayer)
{
  return ASC_setTransportLayer(net_, tLayer, 0);
}

OFCondition DcmFindSCU::dropNetwork()
{
  if (net_) return ASC_dropNetwork(&net_); else return EC_Normal;
}

OFCondition DcmFindSCU::performQuery(
    const char *peer,
    unsigned int port,
    const char *ourTitle,
    const char *peerTitle,
    const char *abstractSyntax,
    E_TransferSyntax preferredTransferSyntax,
    T_DIMSE_BlockingMode blockMode,
    int dimse_timeout,
    Uint32 maxReceivePDULength,
    OFBool secureConnection,
    OFBool abortAssociation,
    unsigned int repeatCount,
    OFBool extractResponsesToFile,
    int cancelAfterNResponses,
    DcmDataset *overrideKeys,
    DcmFindSCUCallback *callback,
    OFList<OFString> *fileNameList)
{
    T_ASC_Association *assoc = NULL;
    T_ASC_Parameters *params = NULL;
    DIC_NODENAME localHost;
    DIC_NODENAME peerHost;

    /* initialize asscociation parameters, i.e. create an instance of T_ASC_Parameters*. */
    OFCondition cond = ASC_createAssociationParameters(&params, maxReceivePDULength);
    if (cond.bad()) return cond;

    /* sets this application's title and the called application's title in the params */
    /* structure. The default values to be set here are "STORESCU" and "ANY-SCP". */
    ASC_setAPTitles(params, ourTitle, peerTitle, NULL);

    /* Set the transport layer type (type of network connection) in the params */
    /* structure. The default is an insecure connection; where OpenSSL is  */
    /* available the user is able to request an encrypted,secure connection. */
    cond = ASC_setTransportLayerType(params, secureConnection);
    if (cond.bad()) return cond;

    /* Figure out the presentation addresses and copy the */
    /* corresponding values into the association parameters.*/
    gethostname(localHost, sizeof(localHost) - 1);
    sprintf(peerHost, "%s:%d", peer, (int)port);
    ASC_setPresentationAddresses(params, localHost, peerHost);

    /* Set the presentation contexts which will be negotiated */
    /* when the network connection will be established */
    cond = addPresentationContext(params, abstractSyntax, preferredTransferSyntax);

    if (cond.bad()) return cond;

    /* dump presentation contexts if required */
    if (debug_) {
        STD_NAMESPACE ostream& mycout = ofConsole.lockCout();
        mycout << "Request Parameters:\n";
        ASC_dumpParameters(params, mycout);
        ofConsole.unlockCout();
    }

    /* create association, i.e. try to establish a network connection to another */
    /* DICOM application. This call creates an instance of T_ASC_Association*. */
    if (verbose_)
        printf("Requesting Association\n");

    cond = ASC_requestAssociation(net_, params, &assoc);

    if (cond.bad()) {
        if (cond == DUL_ASSOCIATIONREJECTED) {
            T_ASC_RejectParameters rej;
            ASC_getRejectParameters(params, &rej);

            STD_NAMESPACE ostream& mycerr = ofConsole.lockCerr();
            mycerr << "Association Rejected:" << OFendl;
            ASC_printRejectParameters(mycerr, &rej);
            ofConsole.unlockCerr();
            return cond;
        } else {
            ofConsole.lockCerr() << "Association Request Failed:" << OFendl;
            ofConsole.unlockCerr();
            return cond;
        }
    }

    /* dump the presentation contexts which have been accepted/refused */
    if (debug_) {
        STD_NAMESPACE ostream& mycout = ofConsole.lockCout();
        mycout << "Association Parameters Negotiated:\n";
        ASC_dumpParameters(params, mycout);
        ofConsole.unlockCout();
    }

    /* count the presentation contexts which have been accepted by the SCP */
    /* If there are none, finish the execution */
    if (ASC_countAcceptedPresentationContexts(params) == 0) {
        ofConsole.lockCerr() << "No Acceptable Presentation Contexts" << OFendl;
        ofConsole.unlockCerr();
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }

    /* dump general information concerning the establishment of the network connection if required */
    if (verbose_) {
        printf("Association Accepted (Max Send PDV: %lu)\n", assoc->sendPDVLength);
    }

    /* do the real work, i.e. for all files which were specified in the command line, send a */
    /* C-FIND-RQ to the other DICOM application and receive corresponding response messages. */
    cond = EC_Normal;
    if ((fileNameList == NULL) || fileNameList->empty())
    {
        /* no files provided on command line */
        cond = findSCU(assoc, NULL, repeatCount, abstractSyntax, blockMode, dimse_timeout, extractResponsesToFile, cancelAfterNResponses, overrideKeys, callback);
    } else {
      OFListIterator(OFString) iter = fileNameList->begin();
      OFListIterator(OFString) enditer = fileNameList->end();
      while ((iter != enditer) && (cond.good())) // compare with EC_Normal since DUL_PEERREQUESTEDRELEASE is also good()
      {
          cond = findSCU(assoc, (*iter).c_str(), repeatCount, abstractSyntax, blockMode, dimse_timeout, extractResponsesToFile, cancelAfterNResponses, overrideKeys, callback);
          ++iter;
      }
    }

    /* tear down association, i.e. terminate network connection to SCP */
    if (cond == EC_Normal)
    {
        if (abortAssociation) {
            if (verbose_)
                printf("Aborting Association\n");
            cond = ASC_abortAssociation(assoc);
            if (cond.bad()) {
                ofConsole.lockCerr() << "Association Abort Failed:" << OFendl;
                ofConsole.unlockCerr();
                return cond;
            }
        } else {
            /* release association */
            if (verbose_)
                printf("Releasing Association\n");
            cond = ASC_releaseAssociation(assoc);
            if (cond.bad())
            {
                ofConsole.lockCerr() << "Association Release Failed:" << OFendl;
                ofConsole.unlockCerr();
                return cond;
            }
        }
    }
    else if (cond == DUL_PEERREQUESTEDRELEASE)
    {
        ofConsole.lockCerr() << "Protocol Error: peer requested release (Aborting)" << OFendl;
        ofConsole.unlockCerr();
        if (verbose_)
            printf("Aborting Association\n");
        cond = ASC_abortAssociation(assoc);
        if (cond.bad()) {
            ofConsole.lockCerr() << "Association Abort Failed:" << OFendl;
            ofConsole.unlockCerr();
            return cond;
        }
    }
    else if (cond == DUL_PEERABORTEDASSOCIATION)
    {
        if (verbose_) printf("Peer Aborted Association\n");
    }
    else
    {
        ofConsole.lockCerr() << "SCU Failed:" << OFendl;
        ofConsole.unlockCerr();
        DimseCondition::dump(cond);
        if (verbose_)
            printf("Aborting Association\n");
        cond = ASC_abortAssociation(assoc);
        if (cond.bad()) {
            ofConsole.lockCerr() << "Association Abort Failed:" << OFendl;
            ofConsole.unlockCerr();
            return cond;
        }
    }

    /* destroy the association, i.e. free memory of T_ASC_Association* structure. This */
    /* call is the counterpart of ASC_requestAssociation(...) which was called above. */
    cond = ASC_destroyAssociation(&assoc);
    return cond;
}

OFCondition DcmFindSCU::addPresentationContext(
  T_ASC_Parameters *params, 
  const char *abstractSyntax,
  E_TransferSyntax preferredTransferSyntax)
{
    /*
    ** We prefer to use Explicitly encoded transfer syntaxes.
    ** If we are running on a Little Endian machine we prefer
    ** LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
    ** Some SCP implementations will just select the first transfer
    ** syntax they support (this is not part of the standard) so
    ** organise the proposed transfer syntaxes to take advantage
    ** of such behaviour.
    **
    ** The presentation contexts proposed here are only used for
    ** C-FIND and C-MOVE, so there is no need to support compressed
    ** transmission.
    */

    const char* transferSyntaxes[] = { NULL, NULL, NULL };
    int numTransferSyntaxes = 0;

    switch (preferredTransferSyntax) {
    case EXS_LittleEndianImplicit:
        /* we only support Little Endian Implicit */
        transferSyntaxes[0]  = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 1;
        break;
    case EXS_LittleEndianExplicit:
        /* we prefer Little Endian Explicit */
        transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
    case EXS_BigEndianExplicit:
        /* we prefer Big Endian Explicit */
        transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
    default:
        /* We prefer explicit transfer syntaxes.
         * If we are running on a Little Endian machine we prefer
         * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
         */
        if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
        {
            transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        } else {
            transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        }
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
    }

    return ASC_addPresentationContext(
        params, 1, abstractSyntax,
        transferSyntaxes, numTransferSyntaxes);
}

void DcmFindSCU::substituteOverrideKeys(const DcmDataset *overrideKeys, DcmDataset *dset) const
{
    if (overrideKeys == NULL) {
        return; /* nothing to do */
    }

    /* copy the override keys */
    DcmDataset keys(*overrideKeys);

    /* put the override keys into dset replacing existing tags */
    unsigned long elemCount = keys.card();
    for (unsigned long i=0; i<elemCount; i++) {
        DcmElement *elem = keys.remove((unsigned long)0);
        dset->insert(elem, OFTrue);
    }
}

OFBool DcmFindSCU::writeToFile(const char* ofname, DcmDataset *dataset)
{
    /* write out as a file format */

    DcmFileFormat fileformat(dataset); // copies dataset
    OFCondition ec = fileformat.error();
    if (ec.bad()) {
        ofConsole.lockCerr() << "error writing file: " << ofname << ": " << ec.text() << OFendl;
        ofConsole.unlockCerr();
        return OFFalse;
    }

    ec = fileformat.saveFile(ofname, dataset->getOriginalXfer());
    if (ec.bad()) {
        ofConsole.lockCerr() << "error writing file: " << ofname << ": " << ec.text() << OFendl;
        ofConsole.unlockCerr();
        return OFFalse;
    }

    return OFTrue;
}


OFCondition DcmFindSCU::findSCU(
  T_ASC_Association * assoc, 
  const char *fname, 
  int repeatCount,  
  const char *abstractSyntax,
  T_DIMSE_BlockingMode blockMode, 
  int dimse_timeout, 
  OFBool extractResponsesToFile,
  int cancelAfterNResponses,
  DcmDataset *overrideKeys,
  DcmFindSCUCallback *callback) const
    /*
     * This function will read all the information from the given file
     * (this information specifies a search mask), figure out a corresponding
     * presentation context which will be used to transmit a C-FIND-RQ message
     * over the network to the SCP, and it will finally initiate the transmission
     * of data to the SCP.
     *
     * Parameters:
     *   assoc - [in] The association (network connection to another DICOM application).
     *   fname - [in] Name of the file which shall be processed.
     */
{
    T_ASC_PresentationContextID presId;
    T_DIMSE_C_FindRQ req;
    T_DIMSE_C_FindRSP rsp;
    DcmFileFormat dcmff;

    /* if there is a valid filename */
    if (fname != NULL) {

        /* read information from file (this information specifies a search mask). After the */
        /* call to DcmFileFormat::read(...) the information which is encapsulated in the file */
        /* will be available through the DcmFileFormat object. In detail, it will be available */
        /* through calls to DcmFileFormat::getMetaInfo() (for meta header information) and */
        /* DcmFileFormat::getDataset() (for data set information). */
        OFCondition cond = dcmff.loadFile(fname);

        /* figure out if an error occured while the file was read*/
        if (cond.bad()) {
            ofConsole.lockCerr() << "Bad DICOM file: " << fname << ": " << cond.text() << OFendl;
            ofConsole.unlockCerr();
            return cond;
        }
    }

    /* replace specific keys by those in overrideKeys */
    substituteOverrideKeys(overrideKeys, dcmff.getDataset());

    /* figure out which of the accepted presentation contexts should be used */
    presId = ASC_findAcceptedPresentationContextID(assoc, abstractSyntax);

    if (presId == 0) {
        ofConsole.lockCerr() << "No presentation context" << OFendl;
        ofConsole.unlockCerr();
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }

    /* repeatCount specifies how many times a certain file shall be processed */
    int n = repeatCount;

    /* prepare C-FIND-RQ message */
    bzero((char*)&req, sizeof(req));
    strcpy(req.AffectedSOPClassUID, abstractSyntax);
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = DIMSE_PRIORITY_LOW;

    /* prepare the callback data */
    DcmFindSCUDefaultCallback defaultCallback(extractResponsesToFile, cancelAfterNResponses, verbose_);
    if (callback == NULL) callback = &defaultCallback;
    callback->setAssociation(assoc);
    callback->setPresentationContextID(presId);

    /* as long as no error occured and the counter does not equal 0 */
    OFCondition cond = EC_Normal;
    while (cond.good() && n--) 
    {
        DcmDataset *statusDetail = NULL;

        /* complete preparation of C-FIND-RQ message */
        req.MessageID = assoc->nextMsgID++;
        
        /* if required, dump some more general information */
        if (verbose_) {
            STD_NAMESPACE ostream& mycout = ofConsole.lockCout();
            mycout << "Find SCU RQ: MsgID " << req.MessageID << "\nREQUEST:\n";
            dcmff.getDataset()->print(mycout);
            mycout << "--------" << OFendl;
            ofConsole.unlockCout();
        }
        
        /* finally conduct transmission of data */
        OFCondition cond = DIMSE_findUser(assoc, presId, &req, dcmff.getDataset(),
            progressCallback, callback, blockMode, dimse_timeout,
            &rsp, &statusDetail);
        
        /* dump some more general information */
        if (cond.good()) {
            if (verbose_) {
                DIMSE_printCFindRSP(stdout, &rsp);
            } else {
                if (rsp.DimseStatus != STATUS_Success) {
                    printf("Response: %s\n", DU_cfindStatusString(rsp.DimseStatus));
                }
            }
        } else {
            if (fname) {
                ofConsole.lockCerr() << "Find Failed, file: " << fname << ":" << OFendl;
                ofConsole.unlockCerr();
            } else {
                STD_NAMESPACE ostream& mycerr = ofConsole.lockCerr();
                mycerr << "Find Failed, query keys" << OFendl;
                dcmff.getDataset()->print(mycerr);
                ofConsole.unlockCerr();                
            }
            DimseCondition::dump(cond);
        }
        
        /* dump status detail information if there is some */
        if (statusDetail != NULL) {
            STD_NAMESPACE ostream& mycout = ofConsole.lockCout();
            mycout << "  Status Detail:\n";
            statusDetail->print(mycout);
            ofConsole.unlockCout();
            delete statusDetail;
        }
    }

    /* return */
    return cond;
}


/*
 * CVS Log
 * $Log: dfindscu.cc,v $
 * Revision 1.3  2007-10-19 10:56:33  onken
 * Fixed bug in addOverrideKey() that caused  problems when parsing a value in a
 * tag-value combination if the value contained whitespace characters.
 *
 * Revision 1.2  2007/10/18 16:14:34  onken
 * - Fixed bug in addOverrideKey() that caused  problems when parsing a value in a
 * tag-value combination if the value contained whitespace characters.
 *
 * Revision 1.1  2007/02/19 13:13:28  meichel
 * Refactored findscu code into class DcmFindSCU, which is now part of the dcmnet
 *   library, and a short command line tool that only evaluates command line
 *   parameters and then makes use of this class. This facilitates re-use of the
 *   findscu code in other applications.
 *
 *
 */
