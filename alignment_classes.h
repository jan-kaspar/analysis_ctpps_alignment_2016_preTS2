#ifndef _alignment_classes_h_
#define _alignment_classes_h_

#include <cstring>
#include <cstdio>
#include <cmath>
#include <map>
#include <vector>
#include <set>
#include <string>

#include "DataFormats/CTPPSReco/interface/CTPPSLocalTrackLite.h"

#include "DataFormats/CTPPSDetId/interface/TotemRPDetId.h"

//----------------------------------------------------------------------------------------------------

struct AlignmentResult
{
	double sh_x, sh_x_unc;		// mm
	double sh_y, sh_y_unc;		// mm
	double rot_z, rot_z_unc;	// rad

	AlignmentResult(double _sh_x=0., double _sh_x_unc=0., double _sh_y=0., double _sh_y_unc=0., double _rot_z=0., double _rot_z_unc=0.) :
		sh_x(_sh_x), sh_x_unc(_sh_x_unc), sh_y(_sh_y), sh_y_unc(_sh_y_unc), rot_z(_rot_z), rot_z_unc(_rot_z_unc)
	{
	}

	void Write(FILE *f) const
	{
		fprintf(f, "sh_x=%+.3f,sh_x_unc=%+.3f,sh_y=%+.3f,sh_y_unc=%+.3f,rot_z=%+.4f,rot_z_unc=%+.4f\n", sh_x, sh_x_unc, sh_y, sh_y_unc, rot_z, rot_z_unc);
	}

	CTPPSLocalTrackLite Apply(const CTPPSLocalTrackLite &tr) const
	{
		const double x = cos(rot_z) * tr.getX() + sin(rot_z) * tr.getY() + sh_x;
		const double y = -sin(rot_z) * tr.getX() + cos(rot_z) * tr.getY() - sh_y;
		return CTPPSLocalTrackLite(tr.getRPId(), x, 0., y, 0.);
	}
};

//----------------------------------------------------------------------------------------------------

struct AlignmentResults : public std::map<unsigned int, AlignmentResult>
{
	void Write(FILE *f) const
	{
		for (auto &p : *this)
		{
			fprintf(f, "id=%u,", p.first);
			p.second.Write(f);
		}
	}

	int Add(char *line)
	{
		bool idSet = false;
		unsigned int id = 0;
		AlignmentResult result;

		// loop over entries separated by ","
		char *p = strtok(line, ",");
		while (p != NULL)
		{
			// isolate key and value strings
			char *pe = strstr(p, "=");
			if (pe == NULL)
			{
				printf("ERROR in AlignmentResults::Add > entry missing = sign: %s.\n", p);
				return 2;
			}
			
			char *s_key = p;
			
			p = strtok(NULL, ",");

			*pe = 0;

			char *s_val = pe+1;

			// interprete keys
			if (strcmp(s_key, "id") == 0)
			{
				idSet = true;
				id = atoi(s_val);
				continue;
			}

			if (strcmp(s_key, "sh_x") == 0)
			{
				result.sh_x = atof(s_val);
				continue;
			}

			if (strcmp(s_key, "sh_x_unc") == 0)
			{
				result.sh_x_unc = atof(s_val);
				continue;
			}

			if (strcmp(s_key, "sh_y") == 0)
			{
				result.sh_y = atof(s_val);
				continue;
			}

			if (strcmp(s_key, "sh_y_unc") == 0)
			{
				result.sh_y_unc = atof(s_val);
				continue;
			}

			if (strcmp(s_key, "rot_z") == 0)
			{
				result.rot_z = atof(s_val);
				continue;
			}

			if (strcmp(s_key, "rot_z_unc") == 0)
			{
				result.rot_z_unc = atof(s_val);
				continue;
			}

			printf("ERROR in AlignmentResults::Add > unknown key: %s.\n", s_key);
			return 3;
		}

		if (!idSet)
		{
			printf("ERROR in AlignmentResults::Add > id not set on the following line:\n%s.\n", line);
			return 4;
		}

		insert({id, result});

		return 0;
	}

	std::vector<CTPPSLocalTrackLite> Apply(const std::vector<CTPPSLocalTrackLite> &input) const
	{
        std::vector<CTPPSLocalTrackLite> output;

		for (auto &t : input)
		{
			auto ait = find(t.getRPId());
			if (ait == end())
              throw cms::Exception("alignment") << "No alignment data for RP " << t.getRPId();

            output.emplace_back(ait->second.Apply(t));
		}

		return output;
	}
};

//----------------------------------------------------------------------------------------------------

struct AlignmentResultsCollection : public std::map<std::string, AlignmentResults>
{
	int Write(const std::string &fn) const
	{
		FILE *f = fopen(fn.c_str(), "w");
		if (!f)
			return -1;

		Write(f);

		fclose(f);

		return 0;
	}

	void Write(FILE *f) const
	{
		for (auto &p : *this)
		{
			fprintf(f, "\n[%s]\n", p.first.c_str());
			p.second.Write(f);
		}
	}

	int Load(const std::string &fn)
	{
		FILE *f = fopen(fn.c_str(), "r");
		if (!f)
			return -1;

		return Load(f);

		fclose(f);
	}

	int Load(FILE *f)
	{
		std::string label = "unknown";
		AlignmentResults block;

		while (!feof(f))
		{
			char line[300];
			char *success = fgets(line, 300, f);

			if (success == NULL)
				break;

			if (line[0] == '\n')
				continue;

			if (line[0] == '[')
			{
				if (block.size() > 0)
					insert({label, block});

				block.clear();

				char *end = strstr(line, "]");
				if (end == NULL)
				{
					printf("ERROR in AlignmentResultsCollection::Load > missing closing ].\n");
					return 1;
				}

				*end = 0;

				label = line+1;

				continue;
			}

			if (block.Add(line) != 0)
				return 2;
		}

		if (block.size() > 0)
			insert({label, block});

		return 0;
	}
};

#endif
