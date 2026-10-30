#include "proceduralTerrain.hpp"

namespace AC_Engine
{
    ProceduralTerrain::ProceduralTerrain(int width, int height)
    {
        m_width = width;
        m_height = height;
        m_heightmap.resize(m_width * m_height);
        
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                int index = y * m_width + x;
                m_heightmap[index].x = static_cast<float>(x);
                m_heightmap[index].y = 0.0f;
            }
        }
    }

    void ProceduralTerrain::generateTerrain(float scale, float persistence, int octaves, float heightScale)
    {
        if (scale <= 0.0f) scale = 0.005f;
    
        // Somme des amplitudes, pour garder un résultat entre -1 et 1
        float maxAmplitude = 0.0f;
        {
            float a = 1.0f;
            for (int o = 0; o < octaves; ++o) { maxAmplitude += a; a *= persistence; }
        }
    
        // Décalage non entier, pour éviter les zéros au coin (0,0)
        const float offsetX = 37.31f;
        const float offsetY = 91.77f;
    
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                float amplitude = 1.0f;
                float frequency = 1.0f;
                float noiseHeight = 0.0f;
            
                for (int o = 0; o < octaves; ++o)
                {
                    float sampleX = x * scale * frequency + offsetX;
                    float sampleY = y * scale * frequency + offsetY;
                
                    noiseHeight += perlin(sampleX, sampleY) * amplitude;
                    amplitude *= persistence;
                    frequency *= 2.0f;
                }
            
                m_heightmap[y * m_width + x].y = (noiseHeight / maxAmplitude) * heightScale;
            }
        }
    }

    void ProceduralTerrain::applyErosion(int iterations, float erosionFactor)
    {
        // Implementation for applying erosion to the terrain
    }

    void ProceduralTerrain::smoothTerrain(int iterations)
    {
        // Implementation for smoothing the terrain
    }

    void ProceduralTerrain::normalizeTerrain()
    {
        // Implementation for normalizing the terrain height values
    }

    void ProceduralTerrain::saveHeightmap(const std::string& filename) const
    {
        // Implementation for saving the heightmap to a file
    }

    void ProceduralTerrain::generateObj(const std::string& filename)
    {
        std::ofstream outFile(filename);
        if (!outFile.is_open())
        {
            std::cerr << "Erreur : Impossible de creer le fichier OBJ : " << filename << std::endl;
            return;
        }

        float minH = m_heightmap[0].y, maxH = m_heightmap[0].y;
        for (const auto& p : m_heightmap)
        {
            minH = std::min(minH, p.y);
            maxH = std::max(maxH, p.y);
        }
        const float range = std::max(maxH - minH, 0.0001f);

        outFile << "# AC_Engine Procedural Terrain OBJ Export\n";
        outFile << "# Vertices: " << m_width * m_height << "\n";
        outFile << "# Faces: " << (m_width - 1) * (m_height - 1) * 2 << "\n\n";

        // 1. Sommets : v X Y Z  (3D_X = x, 3D_Y = hauteur, 3D_Z = ligne de la grille)
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                const auto& vertex = m_heightmap[y * m_width + x];
                outFile << "v " << vertex.x << " " << vertex.y << " " << static_cast<float>(y) << "\n";
            }
        }
        outFile << "\n";

        // 2. Coordonnées de texture : vt u v  (un vt par sommet, même index que le v)
        for (int y = 0; y < m_height; ++y)
            for (int x = 0; x < m_width; ++x)
            {
                const float t = (m_heightmap[y * m_width + x].y - minH) / range; // 0..1

                if      (t < 0.35f) outFile << "vt 0.25 0.75\n"; // eau
                else if (t < 0.45f) outFile << "vt 0.75 0.75\n"; // sable
                else if (t < 0.65f) outFile << "vt 0.25 0.25\n"; // herbe
                else                outFile << "vt 0.75 0.25\n"; // roche
            }
        outFile << "\n";

        // 3. Faces : f v/vt v/vt v/vt  (indices OBJ base 1)
        for (int y = 0; y < m_height - 1; ++y)
        {
            for (int x = 0; x < m_width - 1; ++x)
            {
                const int vTL = y * m_width + x + 1;
                const int vTR = vTL + 1;
                const int vBL = (y + 1) * m_width + x + 1;
                const int vBR = vBL + 1;

                // Triangle 1 : Haut-Gauche -> Bas-Gauche -> Haut-Droite
                outFile << "f " << vTL << "/" << vTL << " "
                                << vBL << "/" << vBL << " "
                                << vTR << "/" << vTR << "\n";

                // Triangle 2 : Haut-Droite -> Bas-Gauche -> Bas-Droite
                outFile << "f " << vTR << "/" << vTR << " "
                                << vBL << "/" << vBL << " "
                                << vBR << "/" << vBR << "\n";
            }
        }

        outFile.close();
        std::cout << "Fichier OBJ genere avec succes : " << filename << std::endl;
    }
}