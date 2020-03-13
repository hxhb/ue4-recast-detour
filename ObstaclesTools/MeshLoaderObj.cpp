//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include "MeshLoaderObj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#pragma warning (disable:4996)

rcMeshLoaderObj::rcMeshLoaderObj() :
	m_verts(0),
	m_tris(0),
	m_normals(0),
	m_vertCount(0),
	m_triCount(0)
{
}

rcMeshLoaderObj::~rcMeshLoaderObj()
{
	delete [] m_verts;
	delete [] m_normals;
	delete [] m_tris;
}
		
void rcMeshLoaderObj::addVertex(float x, float y, float z, int& cap)
{
	if (m_vertCount+1 > cap)
	{
		cap = !cap ? 8 : cap*2;
		float* nv = new float[cap*3];
		if (m_vertCount)
			memcpy(nv, m_verts, m_vertCount*3*sizeof(float));
		delete [] m_verts;
		m_verts = nv;
	}
	float* dst = &m_verts[m_vertCount*3];
	*dst++ = x;
	*dst++ = y;
	*dst++ = z;
	m_vertCount++;
}

void rcMeshLoaderObj::addTriangle(int a, int b, int c, int& cap)
{
	if (m_triCount+1 > cap)
	{
		cap = !cap ? 8 : cap*2;
		int* nv = new int[cap*3];
		if (m_triCount)
			memcpy(nv, m_tris, m_triCount*3*sizeof(int));
		delete [] m_tris;
		m_tris = nv;
	}
	int* dst = &m_tris[m_triCount*3];
	*dst++ = a;
	*dst++ = b;
	*dst++ = c;
	m_triCount++;
}

static char* parseRow(char* buf, char* bufEnd, char* row, int len)
{
	bool cont = false;
	bool start = true;
	bool done = false;
	int n = 0;
	while (!done && buf < bufEnd)
	{
		char c = *buf;
		buf++;
		// multirow
		switch (c)
		{
			case '\\':
				cont = true; // multirow
				break;
			case '\n':
				if (start) break;
				done = true;
				break;
			case '\r':
				break;
			case '\t':
			case ' ':
				if (start) break;
			default:
				start = false;
				cont = false;
				row[n++] = c;
				if (n >= len-1)
					done = true;
				break;
		}
	}
	row[n] = '\0';
	return buf;
}

static int parseFace(char* row, int* data, int n, int vcnt)
{
	int j = 0;
	while (*row != '\0')
	{
		// Skip initial white space
		while (*row != '\0' && (*row == ' ' || *row == '\t'))
			row++;
		char* s = row;
		// Find vertex delimiter and terminated the string there for conversion.
		while (*row != '\0' && *row != ' ' && *row != '\t')
		{
			if (*row == '/') *row = '\0';
			row++;
		}
		if (*s == '\0')
			continue;
		int vi = atoi(s);
		data[j++] = vi < 0 ? vi+vcnt : vi-1;
		if (j >= n) return j;
	}
	return j;
}

bool rcMeshLoaderObj::load(const char* filename, rcAdditionalData* data)
{
	char* buf = 0;
	FILE* fp = fopen(filename, "rb");
	if (!fp)
		return false;
	fseek(fp, 0, SEEK_END);
	int bufSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buf = new char[bufSize];
	if (!buf)
	{
		fclose(fp);
		return false;
	}
	fread(buf, bufSize, 1, fp);
	fclose(fp);

	char* src = buf;
	char* srcEnd = buf + bufSize;
	char row[512];
	int face[32];
	float x,y,z;
	int nv;
	int vcap = 0;
	int tcap = 0;
	
	m_UsedExtendedData = 0;
	while (src < srcEnd)
	{
		// Parse one row
		row[0] = '\0';
		src = parseRow(src, srcEnd, row, sizeof(row)/sizeof(char));
		// Skip comments
		if (row[0] == '#') 
		{
			continue;
		}
		if (row[0] == 'v' && row[1] != 'n' && row[1] != 't')
		{
			// Vertex pos
			sscanf(row+1, "%f %f %f", &x, &y, &z);
			addVertex(x, y, z, vcap);
		}
		if (row[0] == 'f')
		{
			// Faces
			nv = parseFace(row+1, face, 32, m_vertCount);
			for (int i = 2; i < nv; ++i)
			{
				const int a = face[0];
				const int b = face[i-1];
				const int c = face[i];
				if (a < 0 || a >= m_vertCount || b < 0 || b >= m_vertCount || c < 0 || c >= m_vertCount)
					continue;
				addTriangle(a, b, c, tcap);
			}
		}
		// parse recast demo related data
		if (row[0] == 'r' && row[1] == 'd' && row[2] == '_' && data != 0)
		{
			m_UsedExtendedData = 1;
			//read bounding box data
			if (row[3] == 'b' && row[4] == 'b' && row[5] == 'o' && row[6] == 'x')
			{
				sscanf(row+7, "%f %f %f %f %f %f", &data->bmin[0], &data->bmin[1], &data->bmin[2], &data->bmax[0], &data->bmax[1], &data->bmax[2]);
			}

			if (row[3] == 'a' && row[4] == 'g' && row[5] == 'h')
			{
				sscanf(row+6, "%f", &data->m_agentHeight);
			}

			if (row[3] == 'a' && row[4] == 'g' && row[5] == 'r')
			{
				sscanf(row+6, "%f", &data->m_agentRadius);
			}

			if (row[3] == 'c' && row[4] == 's')
			{
				sscanf(row+5, "%f", &data->cs);
			}

			if (row[3] == 'c' && row[4] == 'h')
			{
				sscanf(row+5, "%f", &data->ch);
			}

			if (row[3] == 'a' && row[4] == 'm' && row[5] == 'c')
			{
				sscanf(row+6, "%d", &data->walkableClimb);
			}

			if (row[3] == 'a' && row[4] == 'm' && row[5] == 's')
			{
				sscanf(row+6, "%f", &data->walkableSlopeAngle);
			}

			if (row[3] == 'r' && row[4] == 'm' && row[5] == 'i' && row[6] == 's')
			{
				sscanf(row+7, "%d", &data->minRegionArea);
			}

			if (row[3] == 'r' && row[4] == 'm' && row[5] == 'a' && row[6] == 's')
			{
				sscanf(row+7, "%d", &data->mergeRegionArea);
			}

			if (row[3] == 'm' && row[4] == 'e' && row[5] == 'l')
			{
				sscanf(row+6, "%d", &data->maxEdgeLen);
			}

			if (row[3] == 'p' && row[4] == 'v' && row[5] == 'f')
			{
				unsigned int pvf = 0;
				sscanf(row+6, "%d", &pvf);
				data->bPerformVoxelFiltering = pvf;
			}

			if (row[3] == 'g' && row[4] == 'd' && row[5] == 'm')
			{
				unsigned int gdm = 0;
				sscanf(row+6, "%d", &gdm);
				data->bGenerateDetailedMesh = gdm;
			}

			if (row[3] == 'm' && row[4] == 'p')
			{
				unsigned int mp = 0;
				sscanf(row+5, "%d", &mp);
				data->regionPartitioning = RC_REGION_MONOTONE;
			}

			if (row[3] == 'm' && row[4] == 'p' && row[5] == 'p' && row[6] == 't')
			{
				sscanf(row+7, "%d", &data->MaxPolysPerTile);
			}

			if (row[3] == 'm' && row[4] == 'v' && row[5] == 'p' && row[6] == 'p')
			{
				sscanf(row+7, "%d", &data->maxVertsPerPoly);
			}

			if (row[3] == 't' && row[4] == 's')
			{
				sscanf(row+5, "%d", &data->tileSize);
			}

		}
	}

	delete [] buf;

	// Calculate normals.
	m_normals = new float[m_triCount*3];
	for (int i = 0; i < m_triCount*3; i += 3)
	{
		const float* v0 = &m_verts[m_tris[i]*3];
		const float* v1 = &m_verts[m_tris[i+1]*3];
		const float* v2 = &m_verts[m_tris[i+2]*3];
		float e0[3], e1[3];
		for (int j = 0; j < 3; ++j)
		{
			e0[j] = v1[j] - v0[j];
			e1[j] = v2[j] - v0[j];
		}
		float* n = &m_normals[i];
		n[0] = e0[1]*e1[2] - e0[2]*e1[1];
		n[1] = e0[2]*e1[0] - e0[0]*e1[2];
		n[2] = e0[0]*e1[1] - e0[1]*e1[0];
		float d = sqrtf(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
		if (d > 0)
		{
			d = 1.0f/d;
			n[0] *= d;
			n[1] *= d;
			n[2] *= d;
		}
	}
	
	strncpy(m_filename, filename, sizeof(m_filename));
	m_filename[sizeof(m_filename)-1] = '\0';
	
	return true;
}
