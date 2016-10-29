   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                 AGENDA HEADER FILE                  */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*   Provides functionality for examining, manipulating,     */
/*   adding, and removing activations from the agenda.       */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*      6.23: Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*      6.24: Removed CONFLICT_RESOLUTION_STRATEGIES and     */
/*            DYNAMIC_SALIENCE compilation flags.            */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*            Added EnvGetActivationBasisPPForm function.    */
/*                                                           */
/*      6.30: Added salience groups to improve performance   */
/*            with large numbers of activations of different */
/*            saliences.                                     */
/*                                                           */
/*            Borland C (IBM_TBC) and Metrowerks CodeWarrior */
/*            (MAC_MCW, IBM_MCW) are no longer supported.    */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*************************************************************/

#ifndef _H_agenda

#pragma once

#define _H_agenda

#include "ruledef.h"
#include "symbol.h"
#include "match.h"

#define WHEN_DEFINED 0
#define WHEN_ACTIVATED 1
#define EVERY_CYCLE 2

#define MAX_DEFRULE_SALIENCE  10000
#define MIN_DEFRULE_SALIENCE -10000
  
/*******************/
/* DATA STRUCTURES */
/*******************/

struct activation
  {
   struct defrule *theRule;
   struct partialMatch *basis;
   int salience;
   unsigned long long timetag;
   int randomID;
   struct activation *prev;
   struct activation *next;
  };

struct salienceGroup
  {
   int salience;
   struct activation *first;
   struct activation *last;
   struct salienceGroup *next;
   struct salienceGroup *prev;
  };

typedef struct activation ACTIVATION;

#define AGENDA_DATA 17

struct agendaData
  { 
#if DEBUGGING_FUNCTIONS
   unsigned WatchActivations;
#endif
   unsigned long NumberOfActivations;
   unsigned long long CurrentTimetag;
   bool AgendaChanged;
   int SalienceEvaluation;
   int Strategy;
  };

#define AgendaData(theEnv) ((struct agendaData *) GetEnvironmentData(theEnv,AGENDA_DATA))

/****************************************/
/* GLOBAL EXTERNAL FUNCTION DEFINITIONS */
/****************************************/

   void                    AddActivation(void *,void *,void *);
   void                    ClearRuleFromAgenda(void *,void *);
   void                   *EnvGetNextActivation(void *,void *);
   struct partialMatch    *EnvGetActivationBasis(void *,void *);
   const char             *EnvGetActivationName(void *,void *);
   struct defrule         *EnvGetActivationRule(void *,void *);
   int                     EnvGetActivationSalience(void *,void *);
   int                     EnvSetActivationSalience(void *,void *,int); // TBD remove?
   void                    EnvGetActivationPPForm(void *,char *,size_t,void *);
   void                    EnvGetActivationBasisPPForm(void *,char *,size_t,void *);
   bool                    MoveActivationToTop(void *,void *);
   bool                    EnvDeleteActivation(void *,void *);
   bool                    DetachActivation(void *,void *);
   void                    EnvAgenda(void *,const char *,void *);
   void                    RemoveActivation(void *,void *,bool,bool);
   void                    RemoveAllActivations(void *);
   bool                    EnvGetAgendaChanged(void *);
   void                    EnvSetAgendaChanged(void *,bool);
   unsigned long           GetNumberOfActivations(void *);
   int                     EnvGetSalienceEvaluation(void *);
   int                     EnvSetSalienceEvaluation(void *,int);
   void                    EnvRefreshAgenda(void *,void *);
   void                    EnvReorderAgenda(void *,void *);
   void                    InitializeAgenda(void *);
   void                    SetSalienceEvaluationCommand(UDFContext *,CLIPSValue *);
   void                    GetSalienceEvaluationCommand(UDFContext *,CLIPSValue *);
   void                    RefreshAgendaCommand(UDFContext *,CLIPSValue *);
   void                    RefreshCommand(UDFContext *,CLIPSValue *);
   bool                    EnvRefresh(void *,void *);
#if DEBUGGING_FUNCTIONS
   void                    AgendaCommand(UDFContext *,CLIPSValue *);
#endif

#endif





