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

    void ProceduralTerrain::generateTerrain(float scale, float persistence, int octaves)
    {
        // Implementation for generating procedural terrain

        if (scale <= 0.0f) scale = 0.005f;

        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                float amplitude = 1.0f;
                float frequency = 1.0f;
                float noiseHeight = 0.0f;

                for (int o = 0; o < octaves; ++o)
                {
                    float sampleX = static_cast<float>(x) * scale * frequency;
                    float sampleY = static_cast<float>(y) * scale * frequency;

                    float perlinValue = perlin(sampleX, sampleY);
                    noiseHeight += perlinValue * amplitude;

                    amplitude *= persistence;
                    frequency *= 2.0f;
                }

                int index = y * m_width + x;
                m_heightmap[index].y = noiseHeight;
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
        // 1. Ouvrir le fichier en écriture
        std::ofstream outFile(filename);
        if (!outFile.is_open())
        {
            std::cerr << "Erreur : Impossible de creer le fichier OBJ : " << filename << std::endl;
            return;
        }

        // Commentaire d'en-tête (Optionnel)
        outFile << "# AC_Engine Procedural Terrain OBJ Export\n";
        outFile << "# Vertices: " << m_width * m_height << "\n";
        outFile << "# Faces: " << (m_width - 1) * (m_height - 1) * 2 << "\n\n";

        // 2. Écrire tous les sommets (Vertices)
        // Format : v X Y Z
        // Attention : Dans votre structure, vertex.y contient la HAUTEUR. 
        // En 3D conventionnelle, l'axe vertical est souvent Y ou Z. Ici on mappe : 3D_X = x, 3D_Y = hauteur, 3D_Z = grille_y
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                int index = y * m_width + x;
                const auto& vertex = m_heightmap[index];

                // Écriture du sommet : Position X, Hauteur Y, Position Z (la ligne de votre grille)
                outFile << "v " << vertex.x << " " << vertex.y << " " << static_cast<float>(y) << "\n";
            }
        }

        outFile << "\n";

        // 3. Générer les faces (Triangles)
        // Format : f v1 v2 v3
        // Note importante : Les index du format OBJ commencent obligatoirement à 1 !
        for (int y = 0; y < m_height - 1; ++y)
        {
            for (int x = 0; x < m_width - 1; ++x)
            {
                // Calcul des index des 4 coins d'une cellule de la grille (Index C++ à base 0)
                int topLeft     = y * m_width + x;
                int topRight    = topLeft + 1;
                int bottomLeft  = (y + 1) * m_width + x;
                int bottomRight = bottomLeft + 1;

                // Conversion en index OBJ (base 1) en ajoutant +1
                int vTL = topLeft + 1;
                int vTR = topRight + 1;
                int vBL = bottomLeft + 1;
                int vBR = bottomRight + 1;

                // Découpage de la cellule carrée en 2 triangles :
                // Triangle 1 : Haut-Gauche -> Bas-Gauche -> Haut-Droite
                outFile << "f " << vTL << " " << vBL << " " << vTR << "\n";

                // Triangle 2 : Haut-Droite -> Bas-Gauche -> Bas-Droite
                outFile << "f " << vTR << " " << vBL << " " << vBR << "\n";
            }
        }

        outFile.close();
        std::cout << "Fichier OBJ genere avec succes : " << filename << std::endl;
    }
}