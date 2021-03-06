#ifndef BIQUADRATICCOMPOSITESURFACES3_H
#define BIQUADRATICCOMPOSITESURFACES3_H
#include "BiquadraticPatches3.h"
#include <Core/Materials.h>
#include <Core/ShaderPrograms.h>
#include "../Core/Lights.h"


namespace cagd {
    class BiquadraticCompositeSurface3
    {
    public:
        enum Direction{N, NW, W,SW,S,SE,E,NE};

        class PatchAttributes
        {
        public:
            BiquadraticPatch3 *patch;
            TriangulatedMesh3 *mesh;
            Material *material;
            ShaderProgram *shader;
            RowMatrix<GenericCurve3*> *u_isolines,*v_isolines;
            PatchAttributes *neighbours[8]; //nullptr at start


            GLuint index;
            //TO DO ctor, copy const (deep copy), operator= (deep copy), dtor
            PatchAttributes();
            PatchAttributes(const PatchAttributes &rhs);
            PatchAttributes& operator= (const PatchAttributes &rhs);
            ~PatchAttributes();

            //Getter - Setters


        };

        protected:
            std::vector<PatchAttributes> _attributes;
            GLuint _maxPatches;

            // _attributes[i].patch = new BiquadraticPatch3();
            // (*_attributes[i].patch)[j] = p_j; j = 0,1,2,3
            //_attributes[i].patch -> UpdateVertexBufferObjectsOfData();
            //_attributes[i].patch -> RenderData();
            // _attributes[i].image = _attributes[i].patch->GenerateImage(..,..,..);
            //_attributes[i].image->UpdateVertexBufferObjects(..);
            //i_attributes[i].image->RenderDerivatives(..,..);
            //_attributes[i].glColor4fv(&(*_attributes[i].color)[0]);
        public:
            std::vector<Material*>  loaded_materials;
            //Constructors
            BiquadraticCompositeSurface3(GLuint max = 1000);
            BiquadraticCompositeSurface3(const BiquadraticCompositeSurface3 &rhs);
            BiquadraticCompositeSurface3& operator=(const BiquadraticCompositeSurface3 &rhs);
            ~BiquadraticCompositeSurface3();


            GLboolean InsertNewIsolatedPatch(GLuint index,Material& material,
                                             GLuint u_div_point_count = 30, GLuint v_div_point_count = 30,
                                             GLuint u_isoline_count = 30, GLuint v_isoline_count = 30,
                                             GLenum usage_flag = GL_STATIC_DRAW);
            GLboolean ContinueExistingPatch(const size_t &patch_index,Direction direction,std::string mat);

            GLboolean JoinExistingPatches(const size_t &patch_index1, Direction direction1,
                                       const size_t &patch_index2, Direction direction2,std::string mat);
            GLboolean MergeExistingPatches(const size_t &patch_index1, Direction direction1,
                                        const size_t &patch_index2, Direction direction2);
            //render all patches, rendere selected patch, update selected patch

            GLboolean RenderPatches(GLboolean d1 = GL_FALSE, GLboolean u_lines = GL_FALSE, GLboolean v_lines = GL_FALSE, GLboolean polygon = GL_FALSE, GLuint light = 3);

            GLboolean ShiftPatch(GLuint index, GLdouble off_x, GLdouble off_y, GLdouble off_z);

            GLboolean UpdatePatch(GLuint index);

            GLboolean ShiftNeighbours(PatchAttributes &patch, std::vector<PatchAttributes*> visited, GLdouble off_x,GLdouble off_y, GLdouble off_z);

            GLboolean MergerOthers(GLuint index1, GLuint index2, Direction d1, Direction d2, std::vector<PatchAttributes*> visited);

            GLboolean MoveControlPoint(GLuint patch_index, GLuint point_i, GLuint point_j, GLdouble x, GLdouble y, GLdouble z);
            GLboolean MoveControlPointNeighbours(GLuint patch_index, int point_i, int point_j, DCoordinate3 point, std::vector<PatchAttributes*> visited);


            //other setters/getters

            void SetShaderForAll(ShaderProgram &shader);
            void SetShaderByIndex(GLuint index, ShaderProgram &shader);

            void SetMaterialForAll(Material &material);
            void SetMaterialByIndex(GLuint index, Material &material);

            PatchAttributes getPatchAttributes(GLuint index);

            GLuint getPatchIndex(GLuint patch_index);

            //direction converter
            int GetDirectionIndex(Direction direction) const;

            //Neighbour printer
            void PrintNeighbours(GLuint index);

            void clear();

            //File operations
            GLuint ReadSurfaceFromFile(const std::string &file, GLuint index);

            GLboolean SaveSurfaceToFile(const std::string &file);

            //Material to text
            std::string convertMaterialToText(Material* m);



        };
    };
#endif // BIQUADRATICCOMPOSITESURFACES3_H
