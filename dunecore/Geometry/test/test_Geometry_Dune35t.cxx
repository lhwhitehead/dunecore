// test_Geometry_Dune35t.cxx

// David Adams
// OCtober 2016
//
// Test the DUNE 35t geometry including channel mapping.
//
// This test demonstrates how to configure and use the LArSoft Geometry
// service outside the art framework. DUNE geometry and geometry helper
// service are used.
//
// Note the geometry service requires the experiment-specific geometry
// helper with the channel map also be loaded. 

#undef NDEBUG

#include "larcore/Geometry/Geometry.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "dune/ArtSupport/ArtServiceHelper.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"

using std::string;
using std::cout;
using std::endl;
using std::ofstream;
using std::istringstream;
using std::setw;
using std::vector;
using geo::View_t;
using geo::SigType_t;
using geo::CryostatID;
using geo::TPCID;
using geo::PlaneID;
using geo::WireID;
typedef readout::TPCsetID APAID;
using readout::ROPID;
using geo::CryostatGeo;
using geo::TPCGeo;

typedef unsigned int Index;

//**********************************************************************

// Helper classes.

template<class T>
void check(string name, T val) {
  cout << name << ": " << val << endl;
}

template<class T, class V>
void check(string name, T val, V checkval) {
  check(name, val);
  T eval = checkval;
  if ( val != eval ) {
    cout << val << " != " << checkval << endl;
    assert(false);
  }
}

template<class T, class V>
void checkval(string name, T val, V chkval) {
  T eval = chkval;
  if ( val != eval ) {
    cout << name << ": " << val << " != " << chkval << endl;
    assert(false);
  }
}

//**********************************************************************

int test_Geometry_Dune35t(string chanmap ="Dune35tChannelMapAlg", bool dorop =true, Index maxchanprint =10) {
  const string myname = "test_Geometry_Dune35t: ";
  string gname = "dune35t_geo";
  cout << myname << "Starting test" << endl;
#ifdef NDEBUG
  cout << myname << "NDEBUG must be off." << endl;
  abort();
#endif
  string line = "-----------------------------";

  cout << myname << line << endl;
  cout << myname << "Channel map: " << chanmap << endl;
  cout << myname << "     Do ROP: " << dorop << endl;

  cout << myname << line << endl;
  cout << myname << "Create configuration." << endl;
  const char* ofname = "test_Geometry_Dune35t.fcl";
  {
    ofstream fout(ofname);
    fout << "#include \"geometry_dune.fcl\"" << endl;;
    fout << "services.Geometry:                   @local::" + gname << endl;;
    fout << "services.ExptGeoHelperInterface:     @local::dune_geometry_helper" << endl;;
    if ( chanmap.size() ) {
      fout << "services.ExptGeoHelperInterface.ChannelMapClass: " << chanmap << endl;
    }
  }

  cout << myname << line << endl;
  cout << myname << "Fetch art service helper." << endl;
  ArtServiceHelper& myash = ArtServiceHelper::instance();
  myash.setLogLevel(3);

  cout << myname << line << endl;
  cout << myname << "Add services from " << ofname << endl;
  assert( myash.addServices(ofname, true) == 0 );

  cout << myname << line << endl;
  cout << myname << "Display services" << endl;
  myash.print();

  cout << myname << line << endl;
  cout << myname << "Load the services." << endl;
  assert( myash.loadServices() == 1 );
  myash.print();

  cout << myname << line << endl;
  cout << myname << "Get Geometry service." << endl;
  art::ServiceHandle<geo::Geometry> pgeo;

  cout << myname << line << endl;
  check("Default wiggle", pgeo->DefaultWiggle());
  check("Geometry name", pgeo->DetectorName(), "dune35t4apa_v6");
  cout << myname << "ROOT name: " << pgeo->ROOTFile() << endl;
  cout << myname << "GDML name: " << pgeo->GDMLFile() << endl;

  cout << myname << line << endl;
  double xlo, ylo, zlo;
  double xhi, yhi, zhi;
  cout << myname << "World box"  << endl;
  pgeo->WorldBox(&xlo, &ylo, &zlo, &xhi, &yhi, &zhi);
  check("  xlo", xlo);
  check("  ylo", ylo);
  check("  zlo", zlo);
  check("  xhi", xhi);
  check("  yhi", yhi);
  check("  zhi", zhi);

  cout << myname << line << endl;
  check("SurfaceY", pgeo->SurfaceY());

  cout << myname << line << endl;
  if ( 0  ) check("GetWorldVolumeName", pgeo->GetWorldVolumeName());
  check("TotalMass", pgeo->TotalMass());

  cout << myname << line << endl;
  check("CryostatHalfWidth", pgeo->CryostatHalfWidth());
  check("CryostatHalfHeight", pgeo->CryostatHalfHeight());
  check("CryostatLength", pgeo->CryostatLength());
  //check("", pgeo->());

  // Assign the expected values.
  const Index encry = 1;
  const Index entpc = 8;
  const Index enpla = 3;
  View_t eview[encry][entpc][enpla];
  SigType_t esigType[encry][entpc][enpla];
  for ( Index icry=0; icry<encry; ++icry ) {
    for ( Index itpc=0; itpc<entpc; ++itpc ) {
      for ( Index ipla=0; ipla<enpla; ++ipla ) {
        esigType[icry][itpc][ipla] = (ipla < 2) ? geo::kInduction : geo::kCollection;
      }
      eview[icry][itpc][0] = geo::kU;
      eview[icry][itpc][1] = geo::kV;
      eview[icry][itpc][2] = geo::kZ;
    }
  }

  Index enwirPerPlane[entpc][enpla];
  enwirPerPlane[0][0] = 359;
  enwirPerPlane[0][1] = 345;
  enwirPerPlane[0][2] = 112;
  enwirPerPlane[2][0] = 194;
  enwirPerPlane[2][1] = 188;
  enwirPerPlane[2][2] = 112;
  enwirPerPlane[4][0] = 236;
  enwirPerPlane[4][1] = 228;
  enwirPerPlane[4][2] = 112;
  for ( Index ipla=0; ipla<enpla; ++ipla ) enwirPerPlane[1][ipla] = enwirPerPlane[0][ipla];
  for ( Index ipla=0; ipla<enpla; ++ipla ) enwirPerPlane[3][ipla] = enwirPerPlane[2][ipla];
  for ( Index ipla=0; ipla<enpla; ++ipla ) enwirPerPlane[5][ipla] = enwirPerPlane[4][ipla];
  for ( Index ipla=0; ipla<enpla; ++ipla ) enwirPerPlane[6][ipla] = enwirPerPlane[0][ipla];
  for ( Index ipla=0; ipla<enpla; ++ipla ) enwirPerPlane[7][ipla] = enwirPerPlane[0][ipla];
  const Index enapa = 4;
  const Index enrop = 4;
  const Index enchaPerRop[enrop] = {144, 144, 112, 112};
  Index enchaPerApa = 0;
  for ( Index irop=0; irop<enrop; ++irop ) enchaPerApa += enchaPerRop[irop];
  const Index enchatot = enapa*enchaPerApa;
  Index efirstchan[encry][enapa][enrop];
  Index chan = 0;
  for ( Index icry=0; icry<encry; ++icry ) {
    for ( Index iapa=0; iapa<enapa; ++iapa ) {
      for ( Index irop=0; irop<enrop; ++irop ) {
        efirstchan[icry][iapa][irop] = chan;
        chan += enchaPerRop[irop];
      }
    }
  }
  vector<Index> echacry(enchatot, 0);     // Expected cryostat for each channel;
  vector<Index> echaapa(enchatot, 999);   // Expected APA for each channel;
  vector<Index> echarop(enchatot, 999);   // Expected ROP for each channel;
  Index icha = 0;
  for ( Index iapa=0; iapa<enapa; ++iapa ) {
    for ( Index irop=0; irop<enrop; ++irop ) {
      for ( Index kcha=0; kcha<enchaPerRop[irop]; ++kcha ) {
        echaapa[icha] = iapa;
        echarop[icha] = irop;
        ++icha;
      }
    }
  }

  cout << myname << line << endl;
  Index ncry = pgeo->Ncryostats();
  check("Ncryostats", ncry, encry);
  check("MaxTPCs", pgeo->MaxTPCs(), entpc);
  check("MaxPlanes", pgeo->MaxPlanes(), enpla);
  check("TotalNTPC", pgeo->TotalNTPC(), entpc);
  check("Nviews", pgeo->Nviews(), enpla);
  check("Nchannels", pgeo->Nchannels(), enchatot);

  cout << myname << line << endl;
  cout << "Check TPC wire plane counts." << endl;
  for ( Index icry=0; icry<ncry; ++icry ) {
    Index ntpc = pgeo->NTPC(icry);
    cout << "  Cryostat " << icry << " has " << ntpc << " TPCs" << endl;
    for ( Index itpc=0; itpc<ntpc; ++itpc ) {
      Index npla = pgeo->Nplanes(itpc, icry);
      cout << "    TPC " << itpc << " has " << npla << " planes" << endl;
      assert( npla == 3 );
      for ( Index ipla=0; ipla<npla; ++ipla ) {
        Index nwir = pgeo->Nwires(ipla, itpc, icry);
        cout << "      Plane " << ipla << " has " << nwir << " wires" << endl;
        assert( nwir == enwirPerPlane[itpc][ipla] );
      }
    }
  }

  cout << myname << line << endl;
  cout << "Check wire planes." << endl;
  for ( PlaneID plaid : pgeo->IteratePlaneIDs() ) {
    cout << "  Plane " << plaid << endl;
    cout << "    Signal type: " << pgeo->SignalType(plaid) << endl;
    cout << "           View: " << pgeo->View(plaid) << endl;
    assert( pgeo->SignalType(plaid) == esigType[plaid.Cryostat][plaid.TPC][plaid.Plane] );
    assert( pgeo->View(plaid) == eview[plaid.Cryostat][plaid.TPC][plaid.Plane] );
  }

  cout << myname << line << endl;
  cout << "Check channel-wire mapping." << endl;
  Index itpc1Last = TPCID::InvalidID;
  Index ipla1Last = WireID::InvalidID;
  Index nprint = 0;
  Index lastwire[entpc][enpla];
  for ( Index itpc=0; itpc<entpc; ++itpc ) 
    for ( Index ipla=0; ipla<enpla; ++ipla )
      lastwire[itpc][ipla] = 0;
  for ( Index icha=0; icha<enchatot; ++icha ) {
    vector<WireID> wirids = pgeo->ChannelToWire(icha);
    assert( wirids.size() > 0 );
    WireID wirid1 = wirids[0];
    Index itpc1 = wirid1.TPC;
    Index iapa1 = itpc1/2;
    Index ipla1 = wirid1.Plane;
    if ( itpc1 != itpc1Last || ipla1 != ipla1Last ) nprint = 0;
    itpc1Last = itpc1;
    ipla1Last = ipla1;
    bool print = nprint < maxchanprint;
    if ( print ) ++nprint;
    if ( print ) cout << "  Channel " << setw(4) << icha << " has " << wirids.size() << " wires:";
    for ( WireID wirid : wirids ) {
      Index itpc = wirid.TPC;
      Index iapa = itpc/2;
      Index ipla = wirid.Plane;
      Index iwir = wirid.Wire;
      if ( print ) cout << " " << itpc << "-" << ipla << "-"<< iwir;
      if ( iwir > lastwire[itpc][ipla] ) lastwire[itpc][ipla] = iwir;
      assert( iapa == iapa1 );
      assert( ipla == ipla1 );
      checkval("\nPlaneWireToChannel", pgeo->PlaneWireToChannel(wirid), icha );
      assert( pgeo->PlaneWireToChannel(wirid) == icha );
      assert( pgeo->SignalType(icha) == esigType[wirid.Cryostat][wirid.TPC][wirid.Plane] );
      assert( pgeo->View(icha) == eview[wirid.Cryostat][wirid.TPC][wirid.Plane] );
    }
    if ( print ) cout << endl;
  }
  for ( Index itpc=0; itpc<entpc; ++itpc ) {
    for ( Index ipla=0; ipla<enpla; ++ipla ) {
      Index nwir = lastwire[itpc][ipla] + 1;
      cout << "  TPC-plane " << itpc << "-" << ipla << " has " << setw(3) << nwir << " wires" << endl;
      assert( nwir == enwirPerPlane[itpc][ipla] );
    }
  }

  if ( dorop ) {
    cout << myname << line << endl;
    cout << "Check ROP counts and channels." << endl;
    check("MaxROPs", pgeo->MaxROPs(), enrop);
    Index icry = 0;
    for ( CryostatID cryid: pgeo->IterateCryostatIDs() ) {
      Index napa = pgeo->NTPCsets(cryid);
      cout << "  Cryostat " << icry << " has " << napa << " APAs" << endl;
      assert( napa == enapa );
      for ( Index iapa=0; iapa<napa; ++iapa ) {
        APAID apaid(cryid, iapa);
        Index nrop = pgeo->NROPs(apaid);
        cout << "    APA " << iapa << " has " << nrop << " ROPs" << endl;
        assert( nrop == enrop );
        for ( Index irop=0; irop<nrop; ++irop ) {
          ROPID ropid(apaid, irop);
          Index ncha = pgeo->Nchannels(ropid);
          Index icha1 = pgeo->FirstChannelInROP(ropid);
          Index icha2 = icha1 + ncha - 1;
          cout << "      ROP " << irop << " has " << ncha << " channels: ["
               << icha1 << ", " << icha2 << "]" << endl;
          assert( ncha == enchaPerRop[irop] );
          assert( icha1 = efirstchan[icry][iapa][irop] );
        }
      }
      ++icry;
    }
    assert( icry == ncry );
    cout << myname << line << endl;
    cout << "Check channel-ROP mapping." << endl;
    icry = 0;
    for ( Index icha=0; icha<enchatot; ++icha ) {
      ROPID ropid = pgeo->ChannelToROP(icha);
      Index icry = ropid.Cryostat;
      Index iapa = ropid.TPCset;
      Index irop = ropid.ROP;
      assert( icry = echacry[icha] );
      assert( iapa = echaapa[icha] );
      assert( irop = echarop[icha] );
    }
      
  } else {
    cout << myname << line << endl;
    cout << "Skipped APA and ROP tests." << endl;
  }  // end dorop

  cout << myname << line << endl;
  cout << myname << "Check some positions." << endl;
  const CryostatGeo& crygeo = pgeo->Cryostat(0);
  double origin[3] = {0.0};
  double crypos[3] = {0.0};
  crygeo.LocalToWorld(origin, crypos);
  double cxlo = crypos[0] - crygeo.HalfWidth();
  double cxhi = crypos[0] + crygeo.HalfWidth();
  double cylo = crypos[1] - crygeo.HalfHeight();
  double cyhi = crypos[1] + crygeo.HalfHeight();
  double czlo = crypos[2] - 0.5*crygeo.Length();
  double czhi = crypos[2] + 0.5*crygeo.Length();
  cout << "Cryostat limits: "
       << "(" << cxlo << ", " << cylo << ", " << czlo << "), "
       << "(" << cxhi << ", " << cyhi << ", " << czhi << ")" << endl;

  vector<double> zfs = {0.2, 0.3, 0.4, 0.5, 0.6, 0.7 };
  vector<double> yfs = {0.2, 0.5, 0.8};
  vector<double> xfs = {0.1, 0.3, 0.6 };
  double xyz[3] = {0.0};
  int w = 9;
  for ( double zf : zfs ) {
    xyz[2] = czlo + zf*(czhi-czlo);
    for ( double yf : yfs ) {
      xyz[1] = cylo + yf*(cyhi-cylo);
      for ( double xf : xfs ) {
        xyz[0] = cxlo + xf*(cxhi-cxlo);
        TPCID tpcid = pgeo->FindTPCAtPosition(xyz);
        cout << "  (" << setw(w) << xyz[0] << "," << setw(w) << xyz[1] << "," << setw(w) << xyz[2] << "): "
             << tpcid << endl;
        assert( tpcid.Cryostat != CryostatID::InvalidID );
        assert( tpcid.TPC != TPCID::InvalidID );
        //WireId wid = pgeo->NearestWireID(xyz, pid);
      }
    }
  }

  cout << myname << line << endl;
  cout << myname << "Done." << endl;
  return 0;
}

//**********************************************************************

int main(int argc, const char* argv[]) {
  string chanmap = "";
  bool dorop = false;
  Index maxchanprint = 10;
  if ( argc > 1 ) {
    string sarg = argv[1];
    if ( sarg == "-h" ) {
      cout << argv[0] << ": ChannelMapClass []";
      return 0;
    }
    chanmap = sarg;
  }
  if ( argc > 2 ) {
    string sarg = argv[2];
    dorop = sarg == "1" || sarg == "true";
  }
  if ( argc > 3 ) {
    istringstream ssarg(argv[3]);
    ssarg >> maxchanprint;
  }
  test_Geometry_Dune35t(chanmap, dorop, maxchanprint);
  cout << "Tests concluded." << endl;
  ArtServiceHelper::close();
  return 0;
}

//**********************************************************************
