/*
*  This file is part of FshFormat, a file format plug-in for Adobe Photoshop(R)
*  that loads and saves FSH images.
*
*  Copyright (C) 2011, 2012, 2013, 2014, 2015, 2022, 2023 Nicholas Hayes
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "FshFormatPS.h"
#include "DxtComp.h"

static int ScoreDXT(const uint32 (&px)[16], const int nstep, const uint32 col1, const uint32 col2, uint32* pack)
{
    int vec[3];

    const uint8* c1 = reinterpret_cast<const uint8*>(&col1);
    const uint8* c2 = reinterpret_cast<const uint8*>(&col2);

    int colorDistance[3];

    colorDistance[0] = c2[0] - c1[0];
    colorDistance[1] = c2[1] - c1[1];
    colorDistance[2] = c2[2] - c1[2];

    int colorDistanceSquared = ((colorDistance[0] * colorDistance[0]) + (colorDistance[1] * colorDistance[1])) + (colorDistance[2] * colorDistance[2]);

    int score = 0;
    pack[0] = 0L;

    for (int i = 15; i >= 0; i--)
    {
        const uint8* ptr = reinterpret_cast<const uint8*>(px + i);
        int choice = 0;

        vec[0] = ptr[0] - c1[0];
        vec[1] = ptr[1] - c1[1];
        vec[2] = ptr[2] - c1[2];

        int xa2 = ((vec[0] * vec[0]) + (vec[1] * vec[1])) + (vec[2] * vec[2]);
        int xav = ((vec[0] * colorDistance[0]) + (vec[1] * colorDistance[1])) + (vec[2] * colorDistance[2]);
        if (colorDistanceSquared > 0)
        {
            choice = ((nstep * xav) + (colorDistanceSquared >> 1)) / colorDistanceSquared;

            if (choice < 0)
            {
                choice = 0;
            }
            else if (choice > nstep)
            {
                choice = nstep;
            }
        }

        score += (xa2 - (((2 * choice) * xav) / nstep)) + (((choice * choice) * colorDistanceSquared) / (nstep * nstep));
        pack[0] = pack[0] << 2;

        if (choice == nstep)
        {
            pack[0] += 1UL;
        }
        else if (choice > 0)
        {
            pack[0] += static_cast<uint32>(choice + 1);
        }
    }

    return score;
}

static void PackDXT(const uint32 (&px)[16], uint8* dest)
{
    int i, j;
    uint32 col1;
    uint32 col2;
    int nstep = 0;
    int bestErr = 0;
    uint32 bestCol1 = 0L;
    uint32 bestCol2 = 0L;

    uint32 uniqueColors[16];
    int uniqueColorCount = 0;

    for (i = 0; i < 16; i++)
    {
        col1 = px[i] & 0xf8fcf8UL;

        for (j = 0; j < uniqueColorCount; j++)
        {
            if (uniqueColors[j] == col1)
            {
                break;
            }
        }
        if (j == uniqueColorCount)
        {
            uniqueColors[uniqueColorCount++] = col1;
        }
    }

    if (uniqueColorCount == 1)
    {
        bestCol1 = uniqueColors[0];
        bestCol2 = uniqueColors[0];
        bestErr = 0x3e8;
        nstep = 3;
    }
    else
    {
        bestErr = 0x40000000;
        for (i = 0; i < (uniqueColorCount - 1); i++)
        {
            for (j = i + 1; j < uniqueColorCount; j++)
            {
                uint32 dst;
                int err = ScoreDXT(px, 2, uniqueColors[i], uniqueColors[j], &dst);
                if (err < bestErr)
                {
                    bestCol1 = uniqueColors[i];
                    bestCol2 = uniqueColors[j];
                    nstep = 2;
                    bestErr = err;
                }
                err = ScoreDXT(px, 3, uniqueColors[i], uniqueColors[j], &dst);
                if (err < bestErr)
                {
                    bestCol1 = uniqueColors[i];
                    bestCol2 = uniqueColors[j];
                    nstep = 3;
                    bestErr = err;
                }
            }
        }
    }

    uint8* c1 = reinterpret_cast<uint8*>(&col1);
    uint8* c2 = reinterpret_cast<uint8*>(&col2);
    col1 = bestCol1;
    col2 = bestCol2;
    uint16* sPtr = reinterpret_cast<uint16*>(dest);
    sPtr[0] = static_cast<uint16>(((c1[0] >> 3) + ((c1[1] >> 2) << 5)) + ((c1[2] >> 3) << 11));
    sPtr[1] = static_cast<uint16>(((c2[0] >> 3) + ((c2[1] >> 2) << 5)) + ((c2[2] >> 3) << 11));

    if ((sPtr[0] > sPtr[1]) ^ (nstep == 3))
    {
        uint16 temp = sPtr[0];
        sPtr[0] = sPtr[1];
        sPtr[1] = temp;
        bestCol1 = col2;
        bestCol2 = col1;
    }

    ScoreDXT(px, nstep, bestCol1, bestCol2, reinterpret_cast<ULONG*>(dest + 4));
}

void CompressFSHToolDXT1(const BYTE* inData, BYTE* outData, const int width, const int height)
{
    if ((height & 3) == 0 && (width & 3) == 0)
    {
        const int blockHeight = height / 4;
        const int blockWidth = width / 4;
        const int stride = 4 * width;

        ULONG dxtPixels[16];

        for (int y = 0; y < blockHeight; y++)
        {
            const int row = y * 4;
            for (int x = 0; x < blockWidth; x++)
            {
                const int col = x * 16;
                for (int i = 0; i < 4; i++)
                {
                    const int dxtRow = i * 4;
                    const BYTE* p = (inData + ((row + i) * stride)) + col;
                    for (int j = 0; j < 4; j++)
                    {
                        const int ofs = j * 4;
                        dxtPixels[dxtRow + j] = static_cast<ULONG>(((p[ofs] << 16) + (p[ofs + 1] << 8)) + p[ofs + 2]);
                    }
                }

                PackDXT(dxtPixels, outData + (((y * 2) * width) + (x * 8)));
            }
        }
    }
}

void CompressFSHToolDXT3(const BYTE* inData, BYTE* outData, const int width, const int height)
{
    if ((height & 3) == 0 && (width & 3) == 0)
    {
        const int blockHeight = height / 4;
        const int blockWidth = width / 4;
        const int stride = 4 * width;

        ULONG dxtPixels[16];
        int row, col, ofs, dxtRow;

        for (int y = 0; y < blockHeight; y++)
        {
            row = y * 4;
            for (int x = 0; x < blockWidth; x++)
            {
                col = x * 16;
                for (int i = 0; i < 4; i++)
                {
                    dxtRow = i * 4;
                    const BYTE* p = (inData + ((row + i) * stride)) + col;

                    for (int j = 0; j < 4; j++)
                    {
                        ofs = j * 4;
                        dxtPixels[dxtRow + j] = static_cast<ULONG>(((p[ofs] << 16) + (p[ofs + 1] << 8)) + p[ofs + 2]);
                    }
                }

                PackDXT(dxtPixels, outData + (((((y * 4) * width)) + (x * 16)) + 8));
            }
        }

        for (int y = 0; y < blockHeight; y++)
        {
            row = y * 4;
            dxtRow = row * width;
            for (int x = 0; x < blockWidth; x++)
            {
                ofs = x * 16;
                col = ofs + 3; // get the alpha offset
                for (int i = 0; i < 4; i++)
                {
                    const BYTE* p = (inData + ((row + i) * stride)) + col;
                    BYTE* tgt = outData + (dxtRow + ofs) + i * 2;

                    tgt[0] = static_cast<BYTE>(((p[0] & 0xf0) >> 4) + (p[4] & 0xf0));
                    tgt[1] = static_cast<BYTE>(((p[8] & 0xf0) >> 4) + (p[12] & 0xf0));
                }
            }
        }
    }
}