//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================
#ifndef DDDIGI_DIGIHITATTENUATOREXP_H
#define DDDIGI_DIGIHITATTENUATOREXP_H

// Framework include files
#include <DDDigi/DigiEventAction.h>

/// Namespace for the AIDA detector description toolkit
namespace dd4hep {

  /// Namespace for the Digitization part of the AIDA detector description toolkit
  namespace digi {

    /// Default base class for all Digitizer actions and derivates thereof.
    /**
     *  This is a utility class supporting properties, output and access to
     *  event and run objects through the context.
     *
     *  \author  M.Frank
     *  \version 1.0
     *  \ingroup DD4HEP_DIGITIZATION
     */
    class DigiHitAttenuatorExp : public DigiEventAction   {
    protected:
      /// Property: Input data segment name
      std::string                    m_input_segment  { };
      /// Property: Container names to be loaded
      std::map<std::string, double>  m_container_attenuation  { };
      /// Property: event masks to be handled
      std::vector<int>               m_masks  { };
      /// Property: T0 with respect to central crossing
      double                         m_t0     { 0e0 };

      /// Keys of all containers to be manipulated
      std::map<unsigned long, double> m_attenuation  { };

    protected:
      /// Define standard assignments and constructors
      DDDIGI_DEFINE_ACTION_CONSTRUCTORS(DigiHitAttenuatorExp);
      /// Default destructor
      virtual ~DigiHitAttenuatorExp();
      /// Attenuator callback for single container
      template <typename T> std::size_t attenuate(T* cont, double factor) const;

    public:
      /// Standard constructor
      DigiHitAttenuatorExp(const DigiKernel& kernel, const std::string& nam);
      /// Initialization callback
      virtual void initialize();
      /// Main functional callback
      virtual void execute(DigiContext& context)  const;
    };
  }    // End namespace digi
}      // End namespace dd4hep
#endif // DDDIGI_DIGIHITATTENUATOREXP_H
