#if !defined (__CINT__) || defined (__CLING__)
#include "AliAnalysisManager.h"
#include "AliAnalysisTaskSELc2pKs0fromKFP.h"
#include <TString.h>
#include <TList.h>
#endif

AliAnalysisTaskSELc2pKs0fromKFP* AddTaskLc2pKs0FromKFParticle(TString finname="", Bool_t IsMC=kTRUE, TString cuttype="", Bool_t writeQATree=kTRUE, Bool_t IsAnaLc2Lpi=kFALSE)
{
    Bool_t writeLcRecTree = kTRUE;
    Bool_t writeLcMCGenTree = kFALSE;
    if (IsMC) writeLcMCGenTree = kTRUE;

    // get the manager via the static access member. since it's static, you don't need
    // an instance of the class to call the function
    AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
    if (!mgr) {
        ::Error("AddMyTask", "No analysis manager to connect to.");
        return NULL;
    }

    // get the input event handler, again via a static method. 
    // this handler is part of the managing system and feeds events
    // to your task
    if (!mgr->GetInputEventHandler()) {
        return NULL;
    }

    // check input cut object
    Bool_t stdCuts = kFALSE;
    TFile* fileCuts;
    if ( finname.EqualTo("") ) {
      stdCuts = kTRUE;
    } else {
      fileCuts = TFile::Open(finname.Data());
      if( !fileCuts || (fileCuts && !fileCuts->IsOpen()) ) {
        cout << "Input file not found : check your cut object" << endl;
        return NULL;
      }
    }

    AliRDHFCutsKFP *RDHFCutsKFP = new AliRDHFCutsKFP();
    if (stdCuts) RDHFCutsKFP->SetStandardCutsPP2010();
    else {
      if (!IsAnaLc2Lpi) {
        RDHFCutsKFP = (AliRDHFCutsKFP*)fileCuts->Get("Lc2pKs0AnaCuts");
        RDHFCutsKFP->SetName("Lc2pKs0AnaCuts");
      }
      if (IsAnaLc2Lpi) {
        RDHFCutsKFP = (AliRDHFCutsKFP*)fileCuts->Get("Lc2LpiAnaCuts");
        RDHFCutsKFP->SetName("Lc2LpiAnaCuts");
      }
    }

    if (!RDHFCutsKFP) {
      cout << "Specific AliRDHFCutsKFP not found" << endl;
      return NULL;
    }

    printf("CREATE TASK\n");

    // by default, a file is open for writing. here, we get the filename
    TString fileName = AliAnalysisManager::GetCommonFileName();
    fileName += ":PWGHF_D2H_KFP_";      // create a subfolder in the file
    fileName += cuttype.Data();
    // now we create an instance of your task
    AliAnalysisTaskSELc2pKs0fromKFP* task = new AliAnalysisTaskSELc2pKs0fromKFP("AliAnalysisTaskSELc2pKs0fromKFP", RDHFCutsKFP);

    if(!task) return NULL;
    task->SetMC(IsMC);
    task->SetAnaLc2Lpi(IsAnaLc2Lpi);
    task->SetDebugLevel(1);
    task->SetWriteLcMCGenTree(writeLcMCGenTree);
    task->SetWriteLcTree(writeLcRecTree);
    task->SetWriteLcQATree(writeQATree);
    // weight
    TF1 *weight = new TF1("weight", "expo", 0., 50.);
    weight->SetParameter(0, 0.853544);
    weight->SetParameter(1, -0.325586);
    task->SetWeightFunction(weight);

    // select type of event
//    task->SelectCollisionCandidates(AliVEvent::kAnyINT); // kAnyINT = kMB | kINT7 | kINT5 | kINT8 | kSPI7
    // add your task to the manager
    mgr->AddTask(task);
    // your task needs input: here we connect the manager to your task
    mgr->ConnectInput(task,0,mgr->GetCommonInputContainer());
    // same for the output
    mgr->ConnectOutput(task,1,mgr->CreateContainer(Form("CutsObj_%s", cuttype.Data()), TList::Class(), AliAnalysisManager::kOutputContainer, fileName.Data()));
    mgr->ConnectOutput(task,2,mgr->CreateContainer(Form("Counter_%s", cuttype.Data()), AliNormalizationCounter::Class(), AliAnalysisManager::kOutputContainer, fileName.Data()));
    mgr->ConnectOutput(task,3,mgr->CreateContainer(Form("tree_event_char_%s", cuttype.Data()), TTree::Class(), AliAnalysisManager::kOutputContainer, fileName.Data()));
    mgr->ConnectOutput(task,4,mgr->CreateContainer(Form("tree_Lc_%s", cuttype.Data()), TTree::Class(), AliAnalysisManager::kOutputContainer, fileName.Data()));
    mgr->ConnectOutput(task,5,mgr->CreateContainer(Form("tree_Lc_MCGen_%s", cuttype.Data()), TTree::Class(), AliAnalysisManager::kOutputContainer, fileName.Data()));
    mgr->ConnectOutput(task,6,mgr->CreateContainer(Form("weight_%s", cuttype.Data()), TList::Class(), AliAnalysisManager::kOutputContainer, fileName.Data()));
    mgr->ConnectOutput(task,7,mgr->CreateContainer(Form("tree_Lc_QA_%s", cuttype.Data()), TTree::Class(), AliAnalysisManager::kOutputContainer, fileName.Data()));

    // in the end, this macro returns a pointer to your task. this will be convenient later on
    // when you will run your analysis in an analysis train on grid
    return task;
}
