#include "ElectronID.hh"

#include "edm4hep/utils/vector_utils.h"

#include "edm4eic/ClusterCollection.h"


#include "edm4hep/utils/kinematics.h"
#include <iostream>

#include <Math/LorentzVector.h>
using ROOT::Math::PxPyPzEVector;

ElectronID::ElectronID() {

	mEe = 10.;
	mEh = 166.;
	std::cout << "!!! ElectronID: You have not specified beam energies...defaulting to 10x100 GeV !!!" << std::endl;

//	mEoP_min = 0.9;
	
	mEoP_min = 0.8;
	mEoP_max = 1.2;

	mDeltaH_min = 0.*mEe;
	mDeltaH_min = 5.*mEe;
		
	mIsoR = 0.7;
	mIsoE = 0.9;


	boost = LorentzRotation();

}

ElectronID::ElectronID(double Ee, double Eh) {

	mEe = Ee;
	mEh = Eh;

	mEoP_min = 0.8;
	mEoP_max = 1.2;

	mDeltaH_min = 0.*mEe;
	mDeltaH_min = 5.*mEe;
		
	mIsoR = 0.7;
	mIsoE = 0.9;

	minTrackPoints = 3;
	
	boost = LorentzRotation();
	
}

ElectronID::~ElectronID() {
}


void ElectronID::SetEvent(const podio::Frame* event) {

	mEvent = event;
	eScatIndex = -1;
	hfs_dpt.clear();
	hfs_dpz.clear();
	hfs_de.clear();
	hfs_theta.clear();
	e_det.clear();
        jet_e_det.clear();
        pi_det.clear();
        else_det.clear();
        det_val.clear();

	return;
	
}

/*
edm4eic::ReconstructedParticleCollection ElectronID::FindHadronicFinalState(bool use_mc, int object_id, LorentzRotation boost) {

	edm4eic::ReconstructedParticleCollection meRecon;
	meRecon.setSubsetCollection();

	edm4hep::MCParticleCollection meMiss;
	meMiss.setSubsetCollection();

	auto& rcparts = mEvent->get<edm4eic::ReconstructedParticleCollection>("ReconstructedParticles");

	if ( use_mc )
	{
		edm4hep::MCParticleCollection meMC = GetMCElectron();
		auto& RecoMC = mEvent->get<edm4eic::MCRecoParticleAssociationCollection>("ReconstructedParticleAssociations");
	
		for(const auto& assoc : RecoMC) 
		{
			if(assoc.getSim() != meMC[0] && assoc.getSim().getGeneratorStatus() == 1) 
			{
				PxPyPzEVector v(assoc.getSim().getMomentum().x, assoc.getSim().getMomentum().y, assoc.getSim().getMomentum().z, assoc.getSim().getEnergy());
				PxPyPzEVector u(assoc.getRec().getMomentum().x, assoc.getRec().getMomentum().y, assoc.getRec().getMomentum().z, assoc.getRec().getEnergy());
				PxPyPzEVector c(assoc.getRec().getMomentum().x, assoc.getRec().getMomentum().y, assoc.getRec().getMomentum().z, GetCalorimeterEnergy(assoc.getRec()));
				
				hfs_dpt.push_back((u.Pt()-v.Pt())/v.Pt());
				hfs_dpz.push_back((u.Z()-v.Z())/v.Z());
				hfs_de.push_back((u.E()-v.E())/v.E());
				hfs_theta.push_back(v.Theta()*(180./M_PI));				

				v = boost(v);
				u = boost(u);
				c = boost(c);

				meRecon.push_back(assoc.getRec());
			}
		}
	}
	else
	{ 
		for(const auto& mcp : rcparts) {
			if ( mcp.getObjectID().index != object_id )
				meRecon.push_back(mcp);
		}
	}

	return meRecon;
}
*/

edm4hep::MCParticle ElectronID::GetMC(edm4eic::ReconstructedParticle e_rec) {

	// std::cout << "Getting MC particle associated with reconstructed particle..." << std::endl;

	const auto& RecoMC = static_cast<const edm4eic::MCRecoParticleAssociationCollection&>(*(mEvent->get("ReconstructedParticleAssociations")));
	for(const auto& assoc : RecoMC) {
		if(assoc.getRec() == e_rec)
			return assoc.getSim();
	}

	return edm4hep::MCParticle();
}

int ElectronID::Check_eID(edm4eic::ReconstructedParticle e_rec) {

	edm4hep::MCParticleCollection meMC = GetMCElectron();
	if ( meMC.size() == 0 )
		return 86; // No MC electron found

	const auto& RecoMC = static_cast<const edm4eic::MCRecoParticleAssociationCollection&>(*(mEvent->get("ReconstructedParticleAssociations")));
	for(const auto& assoc : RecoMC) {
		if(assoc.getRec() == e_rec)
		{
		 	if (assoc.getSim() == meMC[0]) 
				return 0;
			else
				return assoc.getSim().getPDG();
		}
	}

	return 86;
}

void ElectronID::CheckClusters() {

	const auto& EcalEndcapNClusters = static_cast<const edm4hep::ClusterCollection&>(*(mEvent->get("EcalEndcapNClusters")));
	const auto& EcalBarrelScFiClusters = static_cast<const edm4hep::ClusterCollection&>(*(mEvent->get("EcalBarrelScFiClusters")));
	const auto& EcalEndcapPClusters = static_cast<const edm4hep::ClusterCollection&>(*(mEvent->get("EcalEndcapPClusters")));

	std::cout << " Number of clusters in EcalEndcapN: " << EcalEndcapNClusters.size() << std::endl;
	std::cout << " Number of clusters in EcalBarrelScFi: " << EcalBarrelScFiClusters.size() << std::endl;
	std::cout << " Number of clusters in EcalEndcapP: " << EcalEndcapPClusters.size() << std::endl;

	return;
}


/*
edm4eic::ReconstructedParticleCollection ElectronID::FindHadronicFinalState(int object_id) {

	// edm4eic::HadronicFinalStateCollection meRecon;
	edm4eic::ReconstructedParticleCollection meRecon;
	meRecon.setSubsetCollection();

	// auto& rcparts = mEvent->get<edm4eic::HadronicFinalStateCollection>("HadronicFinalState");
	const auto& rcparts = static_cast<const edm4eic::ReconstructedParticleCollection&>(*(mEvent->get("ReconstructedParticles")));

	for(const auto& mcp : rcparts) {
		if ( mcp.getObjectID().index != object_id )
			meRecon.push_back(mcp);
	}

	return meRecon;
}
*/

edm4eic::ReconstructedParticleCollection ElectronID::FindHadronicFinalState(int object_id)
{
    edm4eic::ReconstructedParticleCollection meRecon;
    meRecon.setSubsetCollection();

    const auto& rcparts =
    static_cast<const edm4eic::ReconstructedParticleCollection&>(
    *(mEvent->get("ReconstructedParticles")));

    const auto& RecoMC =
    static_cast<const edm4eic::MCRecoParticleAssociationCollection&>(
    *(mEvent->get("ReconstructedParticleAssociations")));

    for(const auto& mcp : rcparts)
    {
        // remove scattered electron
        if(mcp.getObjectID().index == object_id)
            continue;

        meRecon.push_back(mcp);

        // find MC match
        for(const auto& assoc : RecoMC)
        {
            if(assoc.getRec() == mcp)
            {
                auto sim = assoc.getSim();

                // only final-state MC particles
                if(sim.getGeneratorStatus() != 1)
                    continue;

                // reconstructed particle
                PxPyPzEVector reco(
                    mcp.getMomentum().x,
                    mcp.getMomentum().y,
                    mcp.getMomentum().z,
                    mcp.getEnergy()
                );

                // MC truth particle
                PxPyPzEVector truth(
                    sim.getMomentum().x,
                    sim.getMomentum().y,
                    sim.getMomentum().z,
                    sim.getEnergy()
                );

                // -----------------------------
                // Fill residual vectors
                // -----------------------------

                if(truth.Pt() != 0)
                {
                    hfs_dpt.push_back(
                        (reco.Pt() - truth.Pt()) / truth.Pt()
                    );
                }

                if(truth.Pz() != 0)
                {
                    hfs_dpz.push_back(
                        (reco.Pz() - truth.Pz()) / truth.Pz()
                    );
                }

                if(truth.E() != 0)
                {
                    hfs_de.push_back(
                        (reco.E() - truth.E()) / truth.E()
                    );
                }

                hfs_theta.push_back(
                    truth.Theta() * 180.0 / M_PI
                );

                break;
            }
        }
    }

    return meRecon;
}


edm4eic::ReconstructedParticleCollection ElectronID::FindScatteredElectron() {

	// std::cout << "\nFinding scattered electron candidates..." << std::endl;

	// Get all the edm4eic objects needed for electron ID
	const auto& rcparts = static_cast<const edm4eic::ReconstructedParticleCollection&>(*(mEvent->get("ReconstructedParticles")));
	
	// Create collection for storing scattered electron candidates
	// (subset collection of ReconstructedParticleCollection)
	edm4eic::ReconstructedParticleCollection scatteredElectronCandidates;
	scatteredElectronCandidates.setSubsetCollection();

	edm4eic::ReconstructedParticleCollection scatteredElectronCandidates_noEoP;
	scatteredElectronCandidates_noEoP.setSubsetCollection();

	// Loop over all ReconstructedParticles for this event
	for (const auto& reconPart : rcparts) {

		// Require at least one track and one cluster
		if(reconPart.getClusters().size() > 0 && reconPart.getTracks().size() > 0) 
		{
			// Require negative particle
			if(reconPart.getCharge() >= 0) 
				continue;

			int n_track_points = reconPart.getTracks()[0].measurements_size();

			// Calculate rcpart_ member variables for this event
			CalculateParticleValues(reconPart, rcparts);

			// Calculate isolation fraction for this event
			double recon_isoE = rcpart_sum_cluster_E / rcpart_isolation_E;

			// Calculate E/p for this event
			double recon_EoP = rcpart_sum_cluster_E / edm4hep::utils::magnitude(reconPart.getMomentum());

			det_val.push_back({Check_eID(reconPart), n_track_points, recon_EoP, recon_isoE});

			if ( n_track_points < minTrackPoints ) 
				continue;

			if(recon_isoE < mIsoE) 
				continue;

			double trackTheta = edm4hep::utils::anglePolar(reconPart.getMomentum())*(180./M_PI);
			double clusterTheta = GetMomentumVectorFromCluster(reconPart, 0).Theta()*(180./M_PI);

			if ( recon_EoP > mEoP_min && recon_EoP < mEoP_max )
			{
				scatteredElectronCandidates.push_back(reconPart);
				continue;
			}
			else if ( (trackTheta > 158 && trackTheta < 162) || (clusterTheta > 22 && clusterTheta < 33) )
			{
				scatteredElectronCandidates_noEoP.push_back(reconPart);
				continue;
			}
		}
	}	

	// If EoP is found use that, otherwise loosen cuts 
	if (scatteredElectronCandidates.size() > 0)
		return scatteredElectronCandidates;
	else
		return scatteredElectronCandidates_noEoP;
}



edm4hep::MCParticleCollection ElectronID::GetMCHadronicFinalState() {

	edm4hep::MCParticleCollection mhMC;
	mhMC.setSubsetCollection();

//	auto& mcparts = mEvent->get<edm4hep::MCParticleCollection>("MCParticles");

	const auto& mcparts = static_cast<const edm4hep::MCParticleCollection&>(*(mEvent->get("MCParticles")));

	std::vector<edm4hep::MCParticle> mc_hadronic;
	edm4hep::MCParticleCollection meMC = GetMCElectron();
	
	bool found_scattered_e = false; 
	for(const auto& mcp : mcparts) {
	
	/*	if (mcp.getGeneratorStatus() == 1 && mcp.getObjectID().index != meMC[0].getObjectID().index ) 
			mhMC.push_back(mcp);	
	}
*/

	if (mcp.getGeneratorStatus() == 1)
                {
                        if ( meMC.size() == 0 )
                                mhMC.push_back(mcp);
                        else if (mcp.getObjectID().index != meMC[0].getObjectID().index )
                                mhMC.push_back(mcp);
                }
        }

	return mhMC;
}

 
	edm4hep::MCParticleCollection ElectronID::GetMCElectron() {

	edm4hep::MCParticleCollection meMC;
	meMC.setSubsetCollection();
	
	const auto& mcparts = static_cast<const edm4hep::MCParticleCollection&>(*(mEvent->get("MCParticles")));
	if ( eScatIndex != -1 )
		meMC.push_back(mcparts[eScatIndex]);

	////

	for (const auto& mcp : mcparts) 
	{
		if (mcp.getPDG() != 11 || mcp.getGeneratorStatus() != 4) 
			continue;

		std::vector<edm4hep::MCParticle> stack;
		stack.insert(stack.end(), mcp.getDaughters().begin(), mcp.getDaughters().end());

		int shortest_gen = 999;
		int generations = 0;
		edm4hep::MCParticle meMC_candidates;
		bool found_candidate = false;

		while (!stack.empty() ) {
			generations++;
			auto cur = stack.back();
			stack.pop_back();

			if (cur.getPDG() == 11 && cur.getGeneratorStatus() == 1) {
				
				if ( generations < shortest_gen )
				{
					shortest_gen = generations;
					meMC_candidates = cur;
					found_candidate = true;
				}
				break;
			}

			const auto& kids = cur.getDaughters();
			if (!kids.empty()) {
				stack.insert(stack.end(), kids.begin(), kids.end());
			}
		}

		if ( found_candidate )
			meMC.push_back(meMC_candidates);
	}

	return meMC;
}

/*
edm4eic::ReconstructedParticleCollection ElectronID::GetTruthReconElectron() {

	edm4hep::MCParticleCollection meMC = GetMCElectron();
	edm4eic::ReconstructedParticleCollection meRecon;
	meRecon.setSubsetCollection();

	auto& RecoMC = mEvent->get<edm4eic::MCRecoParticleAssociationCollection>("ReconstructedParticleAssociations");

	for(const auto& assoc : RecoMC) {
		if(assoc.getSim() == meMC[0]) {
			meRecon.push_back(assoc.getRec());
			break;
		}
	}

	return meRecon;

}
*/

edm4eic::ReconstructedParticleCollection ElectronID::GetTruthReconElectron() {

	// cout << "New process " << endl;

	const edm4hep::MCParticleCollection meMC = GetMCElectron();
	edm4eic::ReconstructedParticleCollection meRecon;
	meRecon.setSubsetCollection();

	if ( meMC.size() == 0 )
		return meRecon; // No MC electron found

	const auto& RecoMC = static_cast<const edm4eic::MCRecoParticleAssociationCollection&>(*(mEvent->get("ReconstructedParticleAssociations")));

	for(const auto& assoc : RecoMC) 
	{
		auto e_candidat = assoc.getSim();

		if(assoc.getSim() == meMC[0]) {
			meRecon.push_back(assoc.getRec());
			// break;
		}
	}

	const auto& rcparts = static_cast<const edm4eic::ReconstructedParticleCollection&>(*(mEvent->get("ReconstructedParticles")));
	edm4hep::Vector3f lead_pos(meMC[0].getEndpoint().x, meMC[0].getEndpoint().y, meMC[0].getEndpoint().z);
	CheckSurroundingClusters(lead_pos, rcparts);

	// std::cout << meRecon.size() << " reconstructed particles associated with the MC electron." << std::endl;

	return meRecon;
}	

/*
void ElectronID::CalculateParticleValues(const edm4eic::ReconstructedParticle& rcp,
		const edm4eic::ReconstructedParticleCollection& rcparts) {

	rcpart_sum_cluster_E = 0.;
	rcpart_lead_cluster_E = 0.;
	rcpart_isolation_E = 0.;
//	rcpart_deltaH = 0.;  Why it is removed ?

	const edm4eic::Cluster* lead_cluster = nullptr;

	for (const auto& cluster : rcp.getClusters()) {
		rcpart_sum_cluster_E += cluster.getEnergy();
		if(cluster.getEnergy() > rcpart_lead_cluster_E) {
			lead_cluster = &cluster;
			rcpart_lead_cluster_E = cluster.getEnergy();
		}
	}

	if(!lead_cluster) return;

	const auto& lead_pos = lead_cluster->getPosition();
	double lead_eta = edm4hep::utils::eta(lead_pos);
	double lead_phi = edm4hep::utils::angleAzimuthal(lead_pos);

	for (const auto& other_rcp : rcparts) {
		if (&other_rcp == &rcp) continue;  // Skip the same particle

		for (const auto& other_cluster : other_rcp.getClusters()) {

			const auto& other_pos = other_cluster.getPosition();
			double other_eta = edm4hep::utils::eta(other_pos);
			double other_phi = edm4hep::utils::angleAzimuthal(other_pos);

			double d_eta = other_eta - lead_eta;
			double d_phi = other_phi - lead_phi;

			// Adjust d_phi to be in the range (-pi, pi)
			if (d_phi > M_PI) d_phi-=2*M_PI;
			if (d_phi < -M_PI) d_phi+=2*M_PI;

			double dR = std::sqrt(std::pow(d_eta, 2) + std::pow(d_phi, 2));

			// Check if the cluster is within the isolation cone
			if (dR < mIsoR) {
				rcpart_isolation_E += other_cluster.getEnergy();
			}
		}
	}

	return;
}
*/

void ElectronID::CalculateParticleValues(const edm4eic::ReconstructedParticle& rcp,
		const edm4eic::ReconstructedParticleCollection& rcparts) {

	rcpart_sum_cluster_E = 0.;
	rcpart_lead_cluster_E = 0.;
	rcpart_isolation_E = 0.;

	const edm4eic::Cluster* lead_cluster = nullptr;

	for (const auto& cluster : rcp.getClusters()) {
		rcpart_sum_cluster_E += cluster.getEnergy();
		if(cluster.getEnergy() > rcpart_lead_cluster_E) {
			lead_cluster = &cluster;
			rcpart_lead_cluster_E = cluster.getEnergy();
		}
	}

	if(!lead_cluster) return;

	CheckSurroundingClusters(lead_cluster->getPosition(), rcparts);

	return;
}

void ElectronID::CheckSurroundingClusters(const edm4hep::Vector3f& lead_pos,
		const edm4eic::ReconstructedParticleCollection& rcparts) {

	rcpart_isolation_E = 0.;
	rcpart_n_clusters = 0;

	double lead_eta = edm4hep::utils::eta(lead_pos);
	double lead_phi = edm4hep::utils::angleAzimuthal(lead_pos);

	for (const auto& other_rcp : rcparts) {
		for (const auto& other_cluster : other_rcp.getClusters()) {

			const auto& other_pos = other_cluster.getPosition();
			double other_eta = edm4hep::utils::eta(other_pos);
			double other_phi = edm4hep::utils::angleAzimuthal(other_pos);

			double d_eta = other_eta - lead_eta;
			double d_phi = other_phi - lead_phi;

			// Adjust d_phi to be in the range (-pi, pi)
			if (d_phi > M_PI) d_phi-=2*M_PI;
			if (d_phi < -M_PI) d_phi+=2*M_PI;

			double dR = std::sqrt(std::pow(d_eta, 2) + std::pow(d_phi, 2));

			// Check if the cluster is within the isolation cone
			if (dR < mIsoR) {
				rcpart_isolation_E += other_cluster.getEnergy();
				rcpart_n_clusters ++;
			}
		}
	}

	return;
}

void ElectronID::GetEminusPzSum(double &TrackEminusPzSum, double &CalEminusPzSum) {

	const auto& rcparts = static_cast<const edm4eic::ReconstructedParticleCollection&>(*(mEvent->get("ReconstructedParticles")));

	for (const auto& reconPart : rcparts) {

		// Require at least one track and one cluster
		// if(reconPart.getClusters().size() == 0 || reconPart.getTracks().size() == 0) continue;

		if(reconPart.getTracks().size() > 0 )
		{
			int n_track_points = reconPart.getTracks()[0].measurements_size();
			if ( n_track_points < minTrackPoints ) continue;

			PxPyPzEVector vT(reconPart.getMomentum().x, reconPart.getMomentum().y, reconPart.getMomentum().z, reconPart.getEnergy());
			vT = boost(vT);
			TrackEminusPzSum += (vT.E() - vT.Pz());

			if ( reconPart.getClusters().size() > 0 )
			{
				PxPyPzEVector vC(reconPart.getMomentum().x, reconPart.getMomentum().y, reconPart.getMomentum().z, GetCalorimeterEnergy(reconPart));
				// PxPyPzEVector vC = GetMomentumVectorFromCluster(reconPart, reconPart.getMass());
				// if ( reconPart.getTracks().size() > 0 && n_track_points >= minTrackPoints )
					//  vC.SetPxPyPzE(reconPart.getMomentum().x, reconPart.getMomentum().y, reconPart.getMomentum().z, GetCalorimeterEnergy(reconPart));
				vC = boost(vC);
				CalEminusPzSum += (vC.E() - vC.Pz());
			}
		}
		else if (reconPart.getClusters().size() > 0 )
		{
			PxPyPzEVector vC = GetMomentumVectorFromCluster(reconPart, 0);
			vC = boost(vC);
			CalEminusPzSum += (vC.E() - vC.E()*std::cos(vC.Theta()));
		}
	}

	// std::cout << " recon E - Pz sum: " << reconEminusPzSum << std::endl;

	return;
}

 
edm4eic::ReconstructedParticle ElectronID::SelectHighestPT(const edm4eic::ReconstructedParticleCollection& ecandidates) {

	edm4eic::ReconstructedParticle erec;
	double max_pT = 0.;
	
	for(const auto& ecand : ecandidates) {
		double e_pT = ecand.getTracks().size() > 0 ? edm4hep::utils::magnitudeTransverse(ecand.getMomentum()) : GetMomentumVectorFromCluster(ecand, 0.000511).Pt();
		if(e_pT > max_pT) {
			erec = ecand;
			max_pT = e_pT;
		}
	}

	return erec;
}

/*
double ElectronID::GetClusterCone(const edm4eic::ReconstructedParticle& rcp, double frac=1)
{
	const edm4eic::Cluster* lead_cluster = nullptr;
	double sum_cluster_E = 0.;
    double lead_cluster_E = 0.;
	
	for (const auto& cluster : rcp.getClusters()) {
		sum_cluster_E += cluster.getEnergy();
		if(cluster.getEnergy() > lead_cluster_E) {
			lead_cluster = &cluster;
			lead_cluster_E = cluster.getEnergy();
		}
	}

	if (!lead_cluster)
        return -1.;

	std::vector<std::pair<double, double>> dr_de;

	for (const auto& cluster : rcp.getClusters())
	{
		if (lead_cluster == &cluster)
			continue;

		double d_eta = edm4hep::utils::eta(cluster.getPosition()) - edm4hep::utils::eta(lead_cluster->getPosition());
		double d_phi = edm4hep::utils::angleAzimuthal(cluster.getPosition()) - edm4hep::utils::angleAzimuthal(lead_cluster->getPosition());

		if (d_phi > M_PI) d_phi -= 2 * M_PI;
		if (d_phi < -M_PI) d_phi += 2 * M_PI;

		double dR = std::sqrt(std::pow(d_eta, 2) + std::pow(d_phi, 2));
		dr_de.emplace_back(dR, cluster.getEnergy());
	}

	std::sort(dr_de.begin(), dr_de.end(), [](const auto& left, const auto& right) {return left.first < right.first;});

	return dr_de.back().first; // Return the dR of the farthest cluster 
}

*/

double ElectronID::GetClusterCone(const edm4eic::ReconstructedParticle& rcp, double frac=1)
{
	const edm4eic::Cluster* lead_cluster = nullptr;
	double sum_cluster_E = 0.;
    double lead_cluster_E = 0.;
	
	for (const auto& cluster : rcp.getClusters()) {
		sum_cluster_E += cluster.getEnergy();
		if(cluster.getEnergy() > lead_cluster_E) {
			lead_cluster = &cluster;
			lead_cluster_E = cluster.getEnergy();
		}
	}

	if (!lead_cluster)
        return -1.;

	std::vector<std::pair<double, double>> dr_de;

	for (const auto& cluster : rcp.getClusters())
	{
		if (lead_cluster == &cluster)
			continue;

		double d_eta = edm4hep::utils::eta(cluster.getPosition()) - edm4hep::utils::eta(lead_cluster->getPosition());
		double d_phi = edm4hep::utils::angleAzimuthal(cluster.getPosition()) - edm4hep::utils::angleAzimuthal(lead_cluster->getPosition());

		if (d_phi > M_PI) d_phi -= 2 * M_PI;
		if (d_phi < -M_PI) d_phi += 2 * M_PI;

		double dR = std::sqrt(std::pow(d_eta, 2) + std::pow(d_phi, 2));
		dr_de.emplace_back(dR, cluster.getEnergy());
	}

	std::sort(dr_de.begin(), dr_de.end(), [](const auto& left, const auto& right) {return left.first < right.first;});

	return dr_de.back().first; // Return the dR of the farthest cluster 
}



double ElectronID::GetCalorimeterEnergy(const edm4eic::ReconstructedParticle& rcp) {

	double sum_cluster_E = 0.;
	for (const auto& cluster : rcp.getClusters()) {
		sum_cluster_E += cluster.getEnergy();
	}
	return sum_cluster_E;

}

double ElectronID::GetClusterTheta(const edm4eic::ReconstructedParticle& rcp) {

	const edm4eic::Cluster* lead_cluster = nullptr;
	double lead_E = 0.;

	for (const auto& cluster : rcp.getClusters()) {
		if(cluster.getEnergy() > lead_E) {
			lead_cluster = &cluster;
			lead_E = cluster.getEnergy();
		}
	}

	if(!lead_cluster) return -999.;

	return lead_cluster->getIntrinsicTheta();
}

PxPyPzEVector ElectronID::GetMomentumVectorFromCluster(const edm4eic::ReconstructedParticle& rcp, double mass) {

	const edm4eic::Cluster* lead_cluster = nullptr;
	double sum_cluster_E = 0.;
	double lead_E = 0.;

	for (const auto& cluster : rcp.getClusters()) {
		sum_cluster_E += cluster.getEnergy();
		if(cluster.getEnergy() > lead_E) {
			lead_cluster = &cluster;
			lead_E = cluster.getEnergy();
		}
	}

	if(!lead_cluster)
		cout << "Warning: No clusters found for this reconstructed particle!" << endl;

	if(!lead_cluster) return PxPyPzEVector(0, 0, 0, 0);

	double p = std::sqrt(std::pow(sum_cluster_E, 2) - std::pow(mass, 2));
	double px = p * (lead_cluster->getPosition().x / edm4hep::utils::magnitude(lead_cluster->getPosition()));
	double py = p * (lead_cluster->getPosition().y / edm4hep::utils::magnitude(lead_cluster->getPosition()));
	double pz = p * (lead_cluster->getPosition().z / edm4hep::utils::magnitude(lead_cluster->getPosition()));
	
	return PxPyPzEVector(px, py, pz, sum_cluster_E);
}



/*

void ElectronID::GetBeam(LorentzRotation &boost, TLorentzVector &in_e, TLorentzVector &in_n) 
{ 
    edm4hep::MCParticle mc_electron;
    edm4hep::MCParticle mc_nucleon;

    auto& mcparts = mEvent->get<edm4hep::MCParticleCollection>("MCParticles");
    vector<edm4hep::MCParticle> spec_protons;

	for(const auto& mcp : mcparts)
    {
        if ( mcp.getGeneratorStatus() == 4 )
        {
            if ( mcp.getPDG() == ID_ELECTRON )
                mc_electron = mcp;
            else 
                mc_nucleon = mcp;
        }
    }

    if ( !mc_electron.isAvailable() || !mc_nucleon.isAvailable() )
        return;

	in_e.SetPxPyPzE(mc_electron.getMomentum().x, mc_electron.getMomentum().y, mc_electron.getMomentum().z, mc_electron.getEnergy());
    in_n.SetPxPyPzE(mc_nucleon.getMomentum().x, mc_nucleon.getMomentum().y, mc_nucleon.getMomentum().z, mc_nucleon.getEnergy());

    // get boost matrix -- redo every run because the proton / neutron has different mass .. but really this should not change per event .. to be changed
    const PxPyPzEVector ei(
        eicrecon::round_beam_four_momentum(
            mc_electron.getMomentum(),
            mc_electron.getMass(),
            {-1*mEe},
            0.0)
        );

    const PxPyPzEVector pi(
        eicrecon::round_beam_four_momentum(
            mc_nucleon.getMomentum(),
            mc_nucleon.getMass(),
            {mEh},
            -0.025)
        );
    
    boost = eicrecon::determine_boost(ei, pi); // Get boost to colinear frame

	return;
}

*/
