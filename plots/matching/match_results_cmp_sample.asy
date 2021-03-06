import root;
import pad_layout;
include "../fills_samples.asy";
include "mean_shifts.asy";

string topDir = "../../";

//----------------------------------------------------------------------------------------------------

InitDataSets();

string sample_labels[];
pen sample_pens[];
sample_labels.push("ZeroBias"); sample_pens.push(blue);
sample_labels.push("DoubleEG"); sample_pens.push(red);
sample_labels.push("SingleMuon"); sample_pens.push(heavygreen);

real sfa = 0.3;

string method = "method x";

string ref_label = "data_alig_fill_4828_run_10080";

int rp_ids[];
string rps[], rp_labels[];
real rp_shift_m[];
rp_ids.push(3); rps.push("L_1_F"); rp_labels.push("45-210-fr"); rp_shift_m.push(mean_shift_L_1_F);
rp_ids.push(2); rps.push("L_1_N"); rp_labels.push("45-210-nr"); rp_shift_m.push(mean_shift_L_1_N);
rp_ids.push(102); rps.push("R_1_N"); rp_labels.push("56-210-nr"); rp_shift_m.push(mean_shift_R_1_N);
rp_ids.push(103); rps.push("R_1_F"); rp_labels.push("56-210-fr"); rp_shift_m.push(mean_shift_R_1_F);

yTicksDef = RightTicks(0.2, 0.1);

xSizeDef = 40cm;

//----------------------------------------------------------------------------------------------------

string TickLabels(real x)
{
	if (x >=0 && x < fill_data.length)
	{
		return format("%i", fill_data[(int) x].fill);
	} else {
		return "";
	}
}

xTicksDef = LeftTicks(rotate(90)*Label(""), TickLabels, Step=1, step=0);

//----------------------------------------------------------------------------------------------------

NewPad(false, 1, 1);

AddToLegend("(" + method + ")");

AddToLegend("with margin", mSq+false+3pt+black);
AddToLegend("without margin", mCi+true+3pt+black);

for (int sai : sample_labels.keys)
{
	AddToLegend(sample_labels[sai], sample_pens[sai]);
}

AttachLegend();

//----------------------------------------------------------------------------------------------------

for (int rpi : rps.keys)
{
	write(rps[rpi]);

	NewRow();

	NewPad("fill", "horizontal shift$\ung{mm}$");

	if (rp_shift_m[rpi] != 0)
	{
		real sh = rp_shift_m[rpi], unc = 0.15;
		real fill_min = -1, fill_max = fill_data.length;
		draw((fill_min, sh+unc)--(fill_max, sh+unc), black+dashed);
		draw((fill_min, sh)--(fill_max, sh), black+1pt);
		draw((fill_min, sh-unc)--(fill_max, sh-unc), black+dashed);
		draw((fill_max, sh-2*unc), invisible);
		draw((fill_max, sh+2*unc), invisible);
	}
	
	for (int fdi : fill_data.keys)
	{
		//if (fill_data[fdi].fill != 6263)
		//	continue;

		write(format("    %i", fill_data[fdi].fill));

		int fill = fill_data[fdi].fill; 
		int rp_id = rp_ids[rpi];

		for (int dsi : fill_data[fdi].datasets.keys)
		{
			string dataset = fill_data[fdi].datasets[dsi].tag;

			write("        " + dataset);
	
			mark m = (fill_data[fdi].datasets[dsi].margin) ? mSq+3pt+false : mCi+3pt;
	
			for (int sai : sample_labels.keys)
			{
				string f = topDir + "data/" + dataset + "/" + sample_labels[sai] + "/match.root";	
				RootObject obj = RootGetObject(f, ref_label + "/" + rps[rpi] + "/" + method + "/g_results", error = false);
	
				if (!obj.valid)
					continue;
	
				real ax[] = { 0. };
				real ay[] = { 0. };
				obj.vExec("GetPoint", 0, ax, ay); real bsh = ay[0];
				obj.vExec("GetPoint", 1, ax, ay); real bsh_unc = ay[0];

				if (bsh != bsh || bsh_unc != bsh_unc)
					continue;

				real x = fdi + sai * sfa / (sample_labels.length - 1) - sfa/2;

				bool pointValid = (fabs(bsh) > 0.01);
	
				pen p = sample_pens[sai];
	
				if (pointValid)
				{
					draw((x, bsh), m + p);
					draw((x, bsh-bsh_unc)--(x, bsh+bsh_unc), p);
				}
			}
		}
	}

	real y_cen = rp_shift_m[rpi];
	limits((-1, y_cen-1.3), (fill_data.length, y_cen+1.3), Crop);
	//xlimits(-1, fill_data.length, Crop);

	AttachLegend("{\SetFontSizesXX " + rp_labels[rpi] + "}");
}

//----------------------------------------------------------------------------------------------------

GShipout(hSkip=5mm, vSkip=1mm);
