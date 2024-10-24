#include "TAGMSmearer.h"

//-----------
// tagm_config_t  (constructor)
//-----------
tagm_config_t::tagm_config_t(JEventLoop *loop) {
	// default values
	TAGM_TSIGMA = 0.200;        // ns
	TAGM_FADC_TSIGMA = 0.350;   // ns
	TAGM_NPIX_PER_GEV = 1.e5;

    std::vector<std::map<std::string, double> > quality;
    if (loop->GetCalib("/PHOTON_BEAM/microscope/fiber_quality", quality)) {
	   jout << "/PHOTON_BEAM/microscope/fiber_quality not used for this run" << endl;
    }
    for (int i=0; i < (int)quality.size(); ++i) {
       int rowcolumn = quality[i]["row"]*1000 + quality[i]["column"];
       fiber_quality[rowcolumn] = quality[i]["code"];
    }
}

//-----------
// SmearEvent
//-----------
void TAGMSmearer::SmearEvent(hddm_s::HDDM *record)
{
   hddm_s::MicroChannelList tagms = record->getMicroChannels();
   hddm_s::MicroChannelList::iterator iter;
   for (iter = tagms.begin(); iter != tagms.end(); ++iter) {
      iter->deleteTaggerHits();
      hddm_s::TaggerTruthHitList thits = iter->getTaggerTruthHits();
      hddm_s::TaggerTruthHitList::iterator titer;
      for (titer = thits.begin(); titer != thits.end(); ++titer) {
         int column = *(int*)titer->getAttribute("column");
         int row = *(int*)titer->getAttribute("row");
         if (tagm_config->fiber_quality[column + row*1000] != 1)
            continue;
         // smear the time
         double t = titer->getT();
         double tADC = titer->getT();
         double npe = titer->getDE() * tagm_config->TAGM_NPIX_PER_GEV;
         if(config->SMEAR_HITS) {
         	t += gDRandom.SampleGaussian(tagm_config->TAGM_TSIGMA);
          	tADC += gDRandom.SampleGaussian(tagm_config->TAGM_FADC_TSIGMA);
          	npe = gDRandom.SamplePoisson(titer->getDE() * tagm_config->TAGM_NPIX_PER_GEV);
		 }
         hddm_s::TaggerHitList hits = iter->addTaggerHits();
         hits().setT(t);
         hits().setTADC(tADC);
         hits().setNpe(npe);
      }

      if (config->DROP_TRUTH_HITS)
         iter->deleteTaggerTruthHits();
   }
}
