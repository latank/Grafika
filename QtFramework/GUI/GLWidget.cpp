#include "GLWidget.h"

#if !defined(__APPLE__)
#include <GL/glu.h>
#endif

#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;

#include <QMouseEvent>
#include <Core/Exceptions.h>
#include "../Core/Matrices.h"
#include "../Core/DCoordinates3.h"
#include "../Core/RealSquareMatrices.h"
#include "../Test/TestFunctions.h"
#include "../Core/Materials.h"
#include "../Core/Constants.h"
#include "../Core/HCoordinates3.h"
#include "../Core/Colors4.h"
#include "../Core/Lights.h"
#include "../Biquadratic/BiquadraticCompostieCurves3.h"

namespace cagd
{
//--------------------------------
// special and default constructor
//--------------------------------
GLWidget::GLWidget(QWidget *parent, const QGLFormat &format): QGLWidget(format, parent)
{
   // _timer = new QTimer(this);
   // _timer->setInterval(0);
   // connect(_timer,SIGNAL(timeout()),this,SLOT(_animate()));

    models = RowMatrix<TriangulatedMesh3>(17);
    surfaces = RowMatrix<ParametricSurface3*>(7);
    curves = RowMatrix<ParametricCurve3*>(8);
    _shaders = RowMatrix<ShaderProgram>(5);
    _colors = RowMatrix<Color4*>(10);
}

//--------------------------------------------------------------------------------------
// this virtual function is called once before the first call to paintGL() or resizeGL()
//--------------------------------------------------------------------------------------
void GLWidget::initializeGL()
{

    // creating a perspective projection matrix
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    _aspect = (double)width() / (double)height();
    _z_near = 1.0;
    _z_far  = 1000.0;
    _fovy   = 45.0;

    gluPerspective(_fovy, _aspect, _z_near, _z_far);

    // setting the model view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    _eye[0] = _eye[1] = 0.0; _eye[2] = 6.0;
    _center[0] = _center[1] = _center[2] = 0.0;
    _up[0] = _up[2] = 0.0; _up[1] = 1.0;

    gluLookAt(_eye[0], _eye[1], _eye[2], _center[0], _center[1], _center[2], _up[0], _up[1], _up[2]);

    // enabling the depth test
    glEnable(GL_DEPTH_TEST);

    // setting the background color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // initial values of transformation parameters
    _angle_x = _angle_y = _angle_z = 0.0;
    _trans_x = _trans_y = _trans_z = 0.0;
    _zoom = 1.0;

    // ...

    try
    {
        // initializing the OpenGL Extension Wrangler library
        GLenum error = glewInit();

        if (error != GLEW_OK)
        {
            throw Exception("Could not initialize the OpenGL Extension Wrangler Library!");
        }

        if (!glewIsSupported("GL_VERSION_2_0"))
        {
            throw Exception("Your graphics card is not compatible with OpenGL 2.0+! "
                            "Try to update your driver or buy a new graphics adapter!");
        }

        // create and store your geometry in display lists or vertex buffer objects
        // ...
    }
    catch (Exception &e)
    {
        cout << e << endl;
    }

    _models();
    _load_shaders();
    set_current_point(0);
    arc();
    patch();
    loadColors();
    curve();
    surface();

}

//-----------------------
// the rendering function
//-----------------------
void GLWidget::paintGL()
{
    // clears the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // stores/duplicates the original model view matrix
    glPushMatrix();

    (isLight) ? lightMode = _side_widget->selectLight->currentIndex() : lightMode = 3;

    cout << "LightMode " << lightMode << endl;
    // applying transformations
    glRotatef(_angle_x, 1.0, 0.0, 0.0);
    glRotatef(_angle_y, 0.0, 1.0, 0.0);
    glRotatef(_angle_z, 0.0, 0.0, 1.0);
    glTranslated(_trans_x, _trans_y, _trans_z);
    glScaled(_zoom, _zoom, _zoom);

    if(_homeworkID == 2)
    {

        //Render all arcs;
        if(_d1 && _d2)
        {
            if(_control_net_status)
            {
                _curve->RenderArcs(GL_TRUE,GL_TRUE,GL_TRUE);
            }
            _curve->RenderArcs(GL_TRUE,GL_TRUE);
        }

        else if(_d1)
        {
            if(_control_net_status)
            {
                _curve->RenderArcs(GL_TRUE,GL_FALSE,GL_TRUE);
            }
            _curve->RenderArcs(GL_TRUE);
        }
        else if(_d2)
        {
            if(_control_net_status)
            {
                _curve->RenderArcs(GL_FALSE,GL_TRUE,GL_TRUE);
            }
            _curve->RenderArcs(GL_FALSE,GL_TRUE);
        }
        else
        {
            if(_control_net_status)
            {
                _curve->RenderArcs(GL_FALSE,GL_FALSE,GL_TRUE);
            }
            _curve->RenderArcs();
        }

        drawControlPoints(_pickPhase);

    }

    else if(_homeworkID == 3)
    {
        if(_uisolines && _visolines)
        {
            if(_d1)
            {
                if(_control_net_status)
                {
                    _composite_surface->RenderPatches(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE,lightMode);
                }
                else
                {
                    _composite_surface->RenderPatches(GL_TRUE,GL_TRUE,GL_TRUE,GL_FALSE,lightMode);
                }

            }
            else
            {
                if(_control_net_status)
                {
                    _composite_surface->RenderPatches(GL_FALSE,GL_TRUE,GL_TRUE,GL_TRUE,lightMode);
                }
                else
                {
                     _composite_surface->RenderPatches(GL_FALSE,GL_TRUE,GL_TRUE,GL_FALSE,lightMode);
                }
            }
        }
        else if(_uisolines && !_visolines)
        {
            if(_d1)
            {
                if(_control_net_status)
                {
                    _composite_surface->RenderPatches(GL_TRUE,GL_TRUE,GL_FALSE,GL_TRUE,lightMode);
                }
                else
                {
                    _composite_surface->RenderPatches(GL_TRUE,GL_TRUE,GL_FALSE,GL_FALSE,lightMode);
                }

            }
            else
            {
                if(_control_net_status)
                {
                    _composite_surface->RenderPatches(GL_FALSE,GL_TRUE,GL_FALSE,GL_TRUE,lightMode);
                }
                else
                {
                    _composite_surface->RenderPatches(GL_FALSE,GL_TRUE,GL_FALSE,GL_FALSE,lightMode);
                }
            }
        }
        else if(_visolines && !_uisolines)
        {
            if(_d1)
            {
                if(_control_net_status)
                {
                    _composite_surface->RenderPatches(GL_TRUE,GL_FALSE,GL_TRUE,GL_TRUE,lightMode);
                }
                else
                {
                    _composite_surface->RenderPatches(GL_TRUE,GL_FALSE,GL_TRUE,GL_FALSE,lightMode);
                }

            }
            else
            {
                if(_control_net_status)
                {
                     _composite_surface->RenderPatches(GL_FALSE,GL_FALSE,GL_TRUE,GL_TRUE,lightMode);
                }
                else
                {
                    _composite_surface->RenderPatches(GL_FALSE,GL_FALSE,GL_TRUE,GL_FALSE,lightMode);
                }
            }
        }
        else
        {
            if(_d1)
            {
                if(_control_net_status)
                {
                    _composite_surface->RenderPatches(GL_TRUE,GL_FALSE,GL_FALSE,GL_TRUE,lightMode);
                }
                else
                {
                    _composite_surface->RenderPatches(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE,lightMode);
                }

            }
            else
            {
                if(_control_net_status)
                {
                    _composite_surface->RenderPatches(GL_FALSE,GL_FALSE,GL_FALSE,GL_TRUE,lightMode);
                }
                else
                {
                    _composite_surface->RenderPatches(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE,lightMode);
                }
            }
        }
        drawControlPoints(_pickPhase);
    }

    else if(_homeworkID == 1)
    {
        renderArc();
    }
    else if(_homeworkID == 0)
    {
        glewInit();

        if(_uisolines > 0)
        {
            for(int i = 0; i<_uiso->GetColumnCount();i++)
            {
                (*_uiso)[i]->RenderDerivatives(0,GL_LINE_STRIP);
                if(_d1)
                    (*_uiso)[i]->RenderDerivatives(1,GL_LINES);
            }
        }

        if(_visolines > 0)
        {
            for(int i = 0; i<_viso->GetColumnCount();i++)
            {
                (*_viso)[i]->RenderDerivatives(0,GL_LINE_STRIP);
                if(_d1)
                    (*_viso)[i]->RenderDerivatives(1,GL_LINES);
            }
        }

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_NORMALIZE);

        if(_patch_status != 0)
        {
            if(_before_interpolation)
            {

                MatFBRuby.Apply();
                _before_interpolation->Render();
            }
        }

        if(_control_net_status != 0)
        {
            if(_after_interpolation)
            {
                _patch.RenderData();
                glEnable(GL_BLEND);
                glDepthMask(GL_FALSE);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                MatFBTurquoise.Apply();
                _after_interpolation->Render();
                glDepthMask(GL_TRUE);
                glDisable(GL_BLEND);
                glBlendFunc(GL_ONE,GL_ONE);
            }
        }

        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        glDisable(GL_NORMALIZE);


    }

    /* -- SURFACES------
     *

        DirectionalLight *dl = 0;

        HCoordinate3 direction(0.0,0.0,1.0,1.0);
        Color4 ambient(0.4,0.4,0.4,1.0);
        Color4 diffuse(0.8,0.8,0.8,1.0);
        Color4 specular(1.0,1.0,1.0,1.0);

        dl = new DirectionalLight(GL_LIGHT0,direction,ambient,diffuse,specular);

        if(dl)
        {
            dl->Enable();
            MatFBRuby.Apply();
            _surface->Render();
            dl->Disable();
        }

        if(dl)
        {
            delete dl,dl = 0;
        }
        */
    // pops the current matrix stack, replacing the current matrix with the one below it on the stack,
    // i.e., the original model view matrix is restored
    glPopMatrix();
}

//----------------------------------------------------------------------------
// when the main window is resized one needs to redefine the projection matrix
//----------------------------------------------------------------------------
void GLWidget::resizeGL(int w, int h)
{
    // setting the new size of the rendering context
    glViewport(0, 0, w, h);

    // redefining the projection matrix
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    _aspect = (double)w / (double)h;

    gluPerspective(_fovy, _aspect, _z_near, _z_far);

    // switching back to the model view matrix
    glMatrixMode(GL_MODELVIEW);

    updateGL();
}

//-----------------------------------
// implementation of the public slots
//-----------------------------------

void GLWidget::set_side_widget(SideWidget *w) {
    _side_widget = w;
}

void GLWidget::set_angle_x(int value)
{
    if (_angle_x != value)
    {
        _angle_x = value;
        updateGL();
    }
}

void GLWidget::set_angle_y(int value)
{
    if (_angle_y != value)
    {
        _angle_y = value;
        updateGL();
    }
}

void GLWidget::set_angle_z(int value)
{
    if (_angle_z != value)
    {
        _angle_z = value;
        updateGL();
    }
}

void GLWidget::set_zoom_factor(double value)
{
    if (_zoom != value)
    {
        _zoom = value;
        updateGL();
    }
}

void GLWidget::set_trans_x(double value)
{
    if (_trans_x != value)
    {
        _trans_x = value;
        updateGL();
    }
}

void GLWidget::set_trans_y(double value)
{
    if (_trans_y != value)
    {
        _trans_y = value;
        updateGL();
    }
}
void GLWidget::set_trans_z(double value)
{
    if (_trans_z != value)
    {
        _trans_z = value;
        updateGL();
    }
}

void GLWidget::set_d0(int value)
{
    if(_d0 != value)
    {
        _d0 = value;
        updateGL();
    }
}

void GLWidget::set_d1(int value)
{
    if(_d1 != value)
    {
        _d1 = value;
        updateGL();
    }
}

void GLWidget::set_d2(int value)
{
    if(_d2 != value)
    {
        _d2 = value;
        updateGL();
    }
}

void GLWidget::set_homeworkID(int value)
{

    if(_homeworkID != value)
    {
        _homeworkID = value;
        cout<<endl<<"New value is " << _homeworkID<<endl;
        updateGL();
    }
}

void GLWidget::set_curveID(int value)
{
    if(_curveID != value)
    {
        _curveID = value;
        cout<<_curveID;
        _pc = curves[_curveID];
        if(!_pc) exit(1);

        GLuint div_point_count = 200;
        GLenum usage_flag = GL_STATIC_DRAW;

        _image_of_pc = nullptr;
        _image_of_pc = _pc->GenerateImage(div_point_count,usage_flag);

        if(!_image_of_pc) exit(1);

        if(!_image_of_pc->UpdateVertexBufferObjects(0.33,usage_flag))
        {
            cout << "Could not create the vertex buffer object of the parametric curve!" << endl;
        }
        //initializeGL();
        updateGL();
    }
}

void GLWidget::set_modelID(int value)
{
    if(_modelID != value)
    {
        _modelID = value;
        _model = models(value);
        cout<<"Set model to "<<value;
        //initializeGL();
        updateGL();

    }
}

void GLWidget::set_colorID(int value)
{
    if(_colorID != value)
    {
        _colorID = value;
        updateGL();
    }
}

void GLWidget::set_polyID(int value)
{
    if(_polyID != value)
    {
        _polyID = value;
        updateGL();
    }
}

void GLWidget::set_surfID(int value)
{
    if(_surfID != value)
    {
        _surfID = value;
        _parametricsurface = nullptr;
        //_parametricsurface = new (nothrow) ParametricSurface3(_pd,screw::u_min,screw::u_max,screw::v_min,screw::v_max);

        _parametricsurface = surfaces(_surfID);
        GLuint div_point_count = 200;
        GLenum usage_flag = GL_STATIC_DRAW;

        _surface = nullptr;
        _surface = _parametricsurface->GenerateImage(div_point_count,div_point_count,usage_flag);

        _surface->UpdateVertexBufferObjects(usage_flag);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_NORMALIZE);
        _angle = 0.0;
        updateGL();
    }
}

void GLWidget::_models()
{
    // RowMatrix<TriangulatedMesh3> models(17);
    /*
        if(_mouse.LoadFromOFF("Models/mouse.off",true))
            if(_mouse.UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
            {

                //_timer->start();
            }
        */

    glewInit();


    if(models[0].LoadFromOFF("Models/mouse.off",true))
    {
        if(models[0].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }

    glewInit();

    if(models[1].LoadFromOFF("Models/angel.off",true))
    {
        if(models[1].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);

        }
    }
    glewInit();
    if(models[2].LoadFromOFF("Models/cube.off",true))
    {
        if(models[2].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[3].LoadFromOFF("Models/bird.off",true))
    {
        if(models[3].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[4].LoadFromOFF("Models/bug.off",true))
    {
        if(models[4].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[5].LoadFromOFF("Models/cone.off",true))
    {
        if(models[5].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[6].LoadFromOFF("Models/dragon.off",true))
    {
        if(models[6].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[7].LoadFromOFF("Models/elephant.off",true))
    {
        if(models[7].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            //_timer->start();
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[8].LoadFromOFF("Models/gangster.off",true))
    {
        if(models[8].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[9].LoadFromOFF("Models/goblet.off",true))
    {
        if(models[9].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[10].LoadFromOFF("Models/horse.off",true))
    {
        if(models[10].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[11].LoadFromOFF("Models/icosahedron.off",true))
    {
        if(models[11].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[12].LoadFromOFF("Models/seashell.off",true))
    {
        if(models[12].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[13].LoadFromOFF("Models/space_station.off",true))
    {
        if(models[13].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[14].LoadFromOFF("Models/sphere.off",true))
    {
        if(models[14].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[15].LoadFromOFF("Models/Spot.off",true))
    {
        if(models[15].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }
    glewInit();
    if(models[16].LoadFromOFF("Models/star.off",true))
    {
        if(models[16].UpdateVertexBufferObjects(GL_DYNAMIC_DRAW))
        {
            _angle = 0.0;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
    }

    _model = models[_modelID];
    cout<<"Model set"<<endl;
}

void GLWidget::patch() {
    cout<<"Patches-----\n";
    _patch.SetData(0,0,-2.0,-2.0,0.0);
    _patch.SetData(0,1,-2.0,-1.0,0.0);
    _patch.SetData(0,2,-2.0,1.0,0.0);
    _patch.SetData(0,3,-2.0,2.0,0.0);

    _patch.SetData(1,0,-1.0,-2.0,0.0);
    _patch.SetData(1,1,-1.0,-1.0,2.0);
    _patch.SetData(1,2,-1.0,1.0,2.0);
    _patch.SetData(1,3,-1.0,2.0,0.0);

    _patch.SetData(2,0,1.0,-2.0,0.0);
    _patch.SetData(2,1,1.0,-1.0,2.0);
    _patch.SetData(2,2,1.0,1.0,2.0);
    _patch.SetData(2,3,1.0,2.0,0.0);

    _patch.SetData(3,0,2.0,-2.0,0.0);
    _patch.SetData(3,1,2.0,-1.0,0.0);
    _patch.SetData(3,2,2.0,1.0,0.0);
    _patch.SetData(3,3,2.0,2.0,0.0);

    _patch.UpdateVertexBufferObjectsOfData();
    _uiso = _patch.GenerateUIsoparametricLines(30,1,20);
    _viso = _patch.GenerateVIsoparametricLines(30,1,20);

    for(GLuint i = 0; i < _uiso->GetColumnCount(); i++)
    {
        (*_uiso)[i]->UpdateVertexBufferObjects();
    }

    for(GLuint i = 0; i < _viso->GetColumnCount(); i++)
    {
        (*_viso)[i]->UpdateVertexBufferObjects();
    }

    _before_interpolation = _patch.GenerateImage(30,30);
    if(_before_interpolation)
        _before_interpolation->UpdateVertexBufferObjects();

    //Knot in u-direction

    RowMatrix<GLdouble> u_knot_vector(4);
    u_knot_vector(0) = 0.0;
    u_knot_vector(1) = 1.0/3.0;
    u_knot_vector(2) = 2.0/3.0;
    u_knot_vector(3) = 1.0;

    //Knot in v-direction
    ColumnMatrix<GLdouble> v_knot_vector(4);
    v_knot_vector(0) = 0.0;
    v_knot_vector(1) = 1.0/3.0;
    v_knot_vector(2) = 2.0/3.0;
    v_knot_vector(3) = 1.0;

    //data_points_to_interp
    Matrix<DCoordinate3> data_points_to_interpolate(4,4);
    for(GLuint row = 0; row < 4; row++)
    {
        for(GLuint column = 0; column < 4; ++column)
        {
            _patch.GetData(row,column,data_points_to_interpolate(row,column));
        }
    }

    //Solve the interpolation problem
    if(_patch.UpdateDataForInterpolation(u_knot_vector,v_knot_vector,data_points_to_interpolate))
    {
        _after_interpolation = _patch.GenerateImage(30,30);

        if(_after_interpolation)
            _after_interpolation->UpdateVertexBufferObjects();
    }
}

void GLWidget::_set_spinboxes(int point)
{
    emit xcoordhasChanged(_data_p(point).x());
    emit ycoordhasChanged(_data_p(point).y());
    emit zcoordhasChanged(_data_p(point).z());
}

void GLWidget::set_current_point(int point)
{
    cout<<"\n-----\n In set_current_point \n------\n";
    _currentpoint = point;
    cout<<"Current point changed to : "<< point << endl;
    _set_spinboxes(point);
}

void GLWidget::_set_data_p_x(double off)
{
    if(off != _data_p(_currentpoint).x())
    {
        cout<<endl<<"Offsetting "<<_currentpoint<<"'s x with " << off<<endl;
        _data_p(_currentpoint).x() = off;
        _cc->UpdateDataForInterpolation(_knot,_data_p);
        _cc->UpdateVertexBufferObjectsOfData();

        _image_of_cc = _cc->GenerateImage(_n,200);
        _image_of_cc->UpdateVertexBufferObjects(0.33);
        updateGL();
    }
}

void GLWidget::_set_data_p_y(double off)
{
    if(off != _data_p(_currentpoint).y())
    {
        cout<<endl<<"Offsetting "<<_currentpoint<<"'s y with " << off<<endl;

        _data_p(_currentpoint).y() = off;
        _cc->UpdateDataForInterpolation(_knot,_data_p);
        _cc->UpdateVertexBufferObjectsOfData();

        _image_of_cc = _cc->GenerateImage(_n,200);
        _image_of_cc->UpdateVertexBufferObjects(0.33);

        updateGL();
    }
}

void GLWidget::_set_data_p_z(double off)
{
    if(off != _data_p(_currentpoint).z())
    {
        cout<<endl<<"Offsetting "<<_currentpoint<<"'s z with " << off<<endl;

        _data_p(_currentpoint).z() = off;
        _cc->UpdateDataForInterpolation(_knot,_data_p);
        _cc->UpdateVertexBufferObjectsOfData();

        _image_of_cc = _cc->GenerateImage(_n,200);
        _image_of_cc->UpdateVertexBufferObjects(0.33);


        updateGL();
    }

}

void GLWidget::set_patch_enable(int value)
{
    if(_patch_status != value)
    {
        _patch_status = value;
        updateGL();
    }
}

void GLWidget::set_control_net(int value)
{
    if(_control_net_status != value)
    {
        _control_net_status = value;
        updateGL();
    }
}

void GLWidget::set_uisolines(int value)
{
    if(_uisolines != value)
    {
        _uisolines = value;
        updateGL();
    }
}

void GLWidget::set_visolines(int value)
{
    if(_visolines != value)
    {
        _visolines = value;
        updateGL();
    }
}

void GLWidget::_animate()
{

    _timer->start();

    GLfloat *vertex = _model.MapVertexBuffer(GL_READ_WRITE);
    GLfloat *normal = _model.MapNormalBuffer(GL_READ_ONLY);

    _angle+=DEG_TO_RADIAN;
    if(_angle >= TWO_PI) _angle-=TWO_PI;

    GLfloat scale = sin(_angle)/3000.0;
    for(GLuint i = 0;i<_model.VertexCount();++i)
    {
        for(GLuint coordinate = 0;coordinate < 3; ++coordinate,++vertex,++normal)
            *vertex += scale * (*normal);
    }

    _model.UnmapNormalBuffer();
    _model.UnmapVertexBuffer();

    // updateGL();

}

void GLWidget::_deanimate()
{
    _timer->stop();
}

void GLWidget::set_toon_color_1(double value)
{
    if(_toon_color_1 != value)
    {
        cout<< "ShaderID : "<<_shaderID << endl;
        _toon_color_1 = value;
        if(_shaderID == 2)
        {
            glEnable(GL_BLEND);
            _currentShader.Enable();
            _currentShader.SetUniformVariable4f("default_outline_color",_toon_color_1,_toon_color_2,_toon_color_3,_toon_color_4);


            updateGL();
            glDisable(GL_BLEND);
            // _currentShader.Disable();
        }


    }
}

void GLWidget::set_toon_color_2(double value)
{
    if(_toon_color_2 != value)
    {
        _toon_color_2 = value;
        if(_shaderID == 2)
        {
            glEnable(GL_BLEND);
            _currentShader.Enable();
            _currentShader.SetUniformVariable4f("default_outline_color",_toon_color_1,_toon_color_2,_toon_color_3,_toon_color_4);


            updateGL();
            glDisable(GL_BLEND);
            // _currentShader.Disable();
        }
    }
}

void GLWidget::set_toon_color_3(double value)
{
    if(_toon_color_3 != value)
    {
        _toon_color_3 = value;
        if(_shaderID == 2)
        {
            glEnable(GL_BLEND);
            _currentShader.Enable();
            _currentShader.SetUniformVariable4f("default_outline_color",_toon_color_1,_toon_color_2,_toon_color_3,_toon_color_4);


            updateGL();
            glDisable(GL_BLEND);
            //  _currentShader.Disable();
        }
    }
}

void GLWidget::set_toon_color_4(double value)
{
    if(_toon_color_4 != value)
    {
        _toon_color_4 = value;
        if(_shaderID == 2)
        {
            glEnable(GL_BLEND);
            _currentShader.Enable();
            _currentShader.SetUniformVariable4f("default_outline_color",_toon_color_1,_toon_color_2,_toon_color_3,_toon_color_4);


            updateGL();
            glDisable(GL_BLEND);
            //_currentShader.Disable();
        }
    }
}

void GLWidget::set_scaling_factor(double value)
{
    if(_scale_factor != value)
    {
        _scale_factor = value;
        if(_shaderID == 1)
        {
            _currentShader.Enable();
            _currentShader.SetUniformVariable1f("scale_factor",_scale_factor);
            updateGL();
            //_currentShader.Disable();
        }
    }
}

void GLWidget::set_smoothing(double value)
{
    if(_smoothing != value)
    {
        _smoothing = value;
        if(_shaderID == 1)
        {
            _currentShader.Enable();
            _currentShader.SetUniformVariable1f("smoothing",_smoothing);
            updateGL();
            // _currentShader.Disable();
        }
    }
}

void GLWidget::set_shading(double value)
{
    if(_shading != value)
    {
        _shading = value;
        if(_shaderID == 1)
        {
            _currentShader.Enable();
            _currentShader.SetUniformVariable1f("shading",_shading);
            updateGL();
            // _currentShader.Disable();
        }
    }
}

void GLWidget::_load_shaders()
{
    _shaders(0).InstallShaders("Shaders/directional_light.vert",
                               "Shaders/directional_light.frag",
                               GL_FALSE);
    _shaders(1).InstallShaders("Shaders/reflection_lines.vert",
                               "Shaders/reflection_lines.frag",
                               GL_FALSE);
    _shaders(2).InstallShaders("Shaders/toon.vert",
                               "Shaders/toon.frag",
                               GL_FALSE);
    _shaders(3).InstallShaders("Shaders/two_sided_lighting.vert",
                               "Shaders/two_sided_lighting.frag",
                               GL_FALSE);
}
void GLWidget::set_shader(int value)
{
    if(value != _shaderID)
    {
        _shaderID = value;
        _currentShader = _shaders(value);
        _currentShader.Enable();
        _composite_surface->SetShaderForAll(_currentShader);
        updateGL();

        //_currentShader.Disable();
    }
}

void GLWidget::enable_shader(int value)
{
    if(value != 0)
    {
        _currentShader.Enable();

    }
    else
    {
        _currentShader.Disable();
    }
    updateGL();


}

void GLWidget::renderCyclic()
{
    set_current_point(0);
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    if(_homeworkID == 6)
    {
        for(GLuint i = 0; i < 2 * _n + 1; i++)
        {
            glVertex3dv(&_data_p[i][0]);

        }
    }

    glEnd();
    glPointSize(1.0);

    if(_cc)
    {
        glColor3f(1.0f,1.0f,1.0f);
        _cc->RenderData(GL_LINE_LOOP);
    }

    if(_image_of_cc)
    {
        glColor3f(1.0f,0.0f,0.0f);
        _image_of_cc->RenderDerivatives(0,GL_LINE_STRIP);
        glPointSize(5.0f);

        if(_d1)
        {
            glColor3f(0.0f,0.5f,0.0f);
            _image_of_cc->RenderDerivatives(1,GL_LINES);
            _image_of_cc->RenderDerivatives(1,GL_POINTS);
        }

        if(_d2)
        {
            glColor3f(0.1f,0.5f,0.9f);
            _image_of_cc->RenderDerivatives(2,GL_LINES);
            _image_of_cc->RenderDerivatives(2,GL_POINTS);
        }
    }
    glPointSize(1.0f);
}

void GLWidget::arc()
{
    _arc = new BiquadraticArcs3();
    GLdouble step = TWO_PI / 4.0;


    _data_p.ResizeRows(4);

    _data_p[0] = DCoordinate3(-1.0, 1.0, 0.0);
    _data_p[3] = DCoordinate3(1.0, 1.0, 0.0);
    _data_p[2] = DCoordinate3(1.0, 1.0, 1.0);
    _data_p[1] = DCoordinate3(-1.0, -1.0, 0.0);
    _knot.ResizeRows(4);

    for(GLuint i = 0; i <= 3.0; i++)
    {
        DCoordinate3 &cp = (*_arc)[i];
        GLdouble u = i * step;
        cp[0] = cos(u);
        cp[1] = sin(u);
    }


    GLuint div_point_count = 200;
    GLenum usage_flag = GL_STATIC_DRAW;

    _arc->UpdateVertexBufferObjectsOfData();

    _image_of_arc = _arc->GenerateImage(3,div_point_count);
    if(!_image_of_arc)
    {
        cout<<"Error no _image_of_cc\n ";
    }
    if(!_image_of_arc->UpdateVertexBufferObjects(1.0,usage_flag))
    {
        cout<<"Cannot create vertex buffer";
    }

}

void GLWidget::renderArc()
{


    glPointSize(1.0);

    if(_arc)
    {
        glColor3f(1.0f,1.0f,1.0f);
        _arc->RenderData(GL_LINE_STRIP);
    }

    if(_image_of_arc)
    {
        glColor3f(1.0f,0.0f,0.0f);
        _image_of_arc->RenderDerivatives(0,GL_LINE_STRIP);
        glPointSize(5.0f);

        if(_d1)
        {
            glColor3f(0.0f,0.5f,0.0f);
            _image_of_arc->RenderDerivatives(1,GL_LINES);
            _image_of_arc->RenderDerivatives(1,GL_POINTS);
        }

        if(_d2)
        {
            glColor3f(0.1f,0.5f,0.9f);
            _image_of_arc->RenderDerivatives(2,GL_LINES);
            _image_of_arc->RenderDerivatives(2,GL_POINTS);
        }
    }
    glPointSize(1.0f);
}

void GLWidget::set_derivative_scale(int value)
{
    GLenum usage_flag = GL_STATIC_DRAW;
    GLdouble scale = (GLdouble)value / 100;
    if(_homeworkID == 1 &&  !_image_of_cc->UpdateVertexBufferObjects(scale,usage_flag))
    {
        cout<<"Could not create vertex buffer of cyclic curve!";
    }
    else if(_homeworkID == 2 && !_image_of_pc->UpdateVertexBufferObjects(scale,usage_flag))
    {
        cout<<"Could not create vertex buffer of parametric curve!";
    }
    else if(_homeworkID == 6 && !_image_of_cc->UpdateVertexBufferObjects(scale,usage_flag))
    {
        cout<<"Could not create vertex buffer of interpolated cyclic curve!";
    }
    else if(_homeworkID == 7 && !_image_of_arc->UpdateVertexBufferObjects(scale,usage_flag))
    {
        cout<<"Could not create vertex buffer of arc!";
    }
    updateGL();
}

void processHits(GLint hits, GLuint buffer[])
{
    unsigned int i, j;
    GLuint names, *ptr;

    printf("hits = %d\n", hits);
    ptr = (GLuint *) buffer;
    for (i = 0; i < hits; i++) {  /* for each hit  */
       names = *ptr;
       printf(" number of names for hit = %d\n", names); ptr++;
       printf("  z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
       printf(" z2 is %g\n", (float) *ptr/0x7fffffff); ptr++;
       printf("   the name is ");
       for (j = 0; j < names; j++) {  /* for each name */
          printf("%d ", *ptr); ptr++;
       }
       printf("\n");
    }
}

void GLWidget::drawControlPoints(bool a) {
//    _model = models(11);
//    std::vector<BiquadraticCompositeCurve3::ArcAttributes> _attributes = _curve->get_attributes();
//    for (GLuint i = 0; i < _attributes.size(); i++) {
//        for (GLuint j = 0; j < 4; j++) {
//            DCoordinate3 point = _attributes[i].arc->GetData(j);
//            if (a)
//                glLoadName(j);
//            glPointSize(10.0);
//            glBegin(GL_POINTS);
//                glColor3d(1.0, 0.0, 0.0);
//                glVertex3f(point.x(), point.y(), point.z());
//            glEnd();
//            glPushMatrix();
//            glTranslated(point.x(), point.y(), point.z());
//            glScaled(0.20, 0.20, 0.20);
//            glEnable(GL_LIGHTING);
//            glEnable(GL_LIGHT0);
//            glEnable(GL_NORMALIZE);
//            MatFBPearl.Apply();
//            _model.Render();
//            glDisable(GL_NORMALIZE);
//            glDisable(GL_LIGHTING);
//            glDisable(GL_LIGHT0);
//            glPopMatrix();
//        }
//    }

    if (a)
    {
        glPointSize(10.0);
        glBegin(GL_POINTS);
            glColor3d(1.0, 0.0, 0.0);
            glVertex3dv(_locator);
        glEnd();
        glPointSize(1.0);

        glDisable(GL_DEPTH_TEST);
        glColor3d(1.0, 0.8, 0.0);
        renderText(_locator[0], _locator[1], _locator[2],
                   QString::number(_locator[0]) + ", " +
                   QString::number(_locator[1]) + ", " +
                   QString::number(_locator[2]));
        glEnable(GL_DEPTH_TEST);
    }
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    GLdouble projection_matrix[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);

    glPushMatrix();

        glRotatef(_angle_x, 1.0, 0.0, 0.0);
        glRotatef(_angle_y, 0.0, 1.0, 0.0);
        glRotatef(_angle_z, 0.0, 0.0, 1.0);
        glTranslated(_trans_x, _trans_y, _trans_z);
        glScaled(_zoom, _zoom, _zoom);

        GLdouble model_view_matrix[16];
        glGetDoublev(GL_MODELVIEW_MATRIX, model_view_matrix);

    glPopMatrix();

    glGetIntegerv(GL_VIEWPORT, viewport);

    GLint    win_x = event->x();
    GLint    win_y = viewport[3] - event->y();
    GLdouble win_z;

    glReadPixels(win_x, win_y, 1, 1, GL_DEPTH_COMPONENT, GL_DOUBLE, &win_z);
    gluUnProject(win_x, win_y, win_z, model_view_matrix, projection_matrix, viewport, &_locator[0], &_locator[1], &_locator[2]);
    cout << "Locator: " << _locator[0] << " " << _locator[1] << " " << _locator[2] << endl;

//    glRenderMode(GL_SELECT);
//    glInitNames();
//    glPushName(0);
//    glSelectBuffer(100, selectBuffer);
//    glMatrixMode(GL_PROJECTION);

//    glPushMatrix();
//    glLoadIdentity();

//    gluPickMatrix(_locator[0], _locator[1], 100.0, 100.0, viewport);
//    gluPerspective(_fovy, _aspect, _z_near, _z_far);
    _pickPhase = true;
    updateGL();
//    glPopMatrix();

//    hits = glRenderMode(GL_RENDER);
//    cout << "Hits: " << hits << endl;

//    glMatrixMode(GL_MODELVIEW);
//    _pickPhase = false;
//    updateGL();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    _pickPhase = false;
    updateGL();
}

/* Arc methods */
void GLWidget::add_arc() {
    _side_widget->deleteArcBox->addItem(_side_widget->arcColorBox->currentText());
    _side_widget->selectArc1->addItem(_side_widget->arcColorBox->currentText());
    _side_widget->selectArc2->addItem(_side_widget->arcColorBox->currentText());

    // Material? Shaders?

    // actual adding of arc with ID

    if(_side_widget->arcColorBox->currentText() == "Red")
    {
        _curve->InsertNewIsolatedArc(_curveindex,_colors[0]);
    }
    else if(_side_widget->arcColorBox->currentText() == "Green")
    {
         _curve->InsertNewIsolatedArc(_curveindex,_colors[1]);
    }
    else if(_side_widget->arcColorBox->currentText() == "Blue")
    {
         _curve->InsertNewIsolatedArc(_curveindex,_colors[2]);
    }
    else if(_side_widget->arcColorBox->currentText() == "Yellow")
    {
        _curve->InsertNewIsolatedArc(_curveindex,_colors[3]);
    }
    else if(_side_widget->arcColorBox->currentText() == "Magenta")
    {
        _curve->InsertNewIsolatedArc(_curveindex,_colors[4]);
    }
    else if(_side_widget->arcColorBox->currentText() == "Cyan")
    {
        _curve->InsertNewIsolatedArc(_curveindex,_colors[5]);
    }
    else
    {
        _curve->InsertNewIsolatedArc(_curveindex);
    }

    _curveindex++;

    updateGL();
}

void GLWidget::delete_arc () {
    _side_widget->selectArc1->clear();
    _side_widget->selectArc2->clear();
    _side_widget->deleteArcBox->clear();

    _curve->clear();
    _curveindex = 0;
    updateGL();

    // actual delete of arc with ID
    /* ex.: BiquadraticCompositeCurves3 x;
    *   loop(x.arc(i)) { delete x.arc(i) }
    *
    * x's ID = _side_widget->deleteArcBox->currentText() (Red, Green, ..)
    *       OR _side_widget->deleteArcBox->currentIndex() (0, 1, ..)
    */
}

void GLWidget::do_arc_operation() {
    // If arc ID is string (else, get index)
    string arc1 = _side_widget->selectArc1->currentText().toStdString();
    string arc2 = _side_widget->selectArc2->currentText().toStdString();
    string direction1 = _side_widget->directionArc1->currentText().toStdString();
    string direction2 = _side_widget->directionArc2->currentText().toStdString();

    GLuint operation = _side_widget->operationBox->currentIndex();

    GLuint index1 = -1,index2 = -1;
    index1 = _side_widget->selectArc1->currentIndex();
    index2 = _side_widget->selectArc2->currentIndex();


    cout<<"Index of 1: " << index1 <<" Index of 2: "<<index2<<endl;



    switch (operation) {
    // Move
    case 0: {
        GLuint pointindex = _side_widget->arcPointSelect->currentIndex();
        GLdouble localx = _side_widget->arcxbox->value();
        GLdouble localy = _side_widget->arcybox->value();
        GLdouble localz = _side_widget->arczbox->value();

        _curve->moveControlPointAll(index1, pointindex, localx, localy, localz);
        break;
    }
    //Shift
    case 1: {
        // hopefully mouseevent handler :(

        GLdouble localx = _side_widget->arcxbox->value();
        GLdouble localy = _side_widget->arcybox->value();
        GLdouble localz = _side_widget->arczbox->value();

        _curve->moveOnAllAxis(index1,localx, localy, localz);
        std::vector<BiquadraticCompositeCurve3::ArcAttributes*> visited;
        visited.clear();
        //_curve->shiftArc(index1,localx,localy,localz,visited);
        break;
    }
    // Continue
    case 2: {
        QString colorName = _side_widget->arcColorBox->currentText();
        Color4 *col = getColorFromString(colorName.toStdString());

        if(direction1 == "Right")
            _curve->ContinueExistingArc(index1,BiquadraticCompositeCurve3::RIGHT, col);
        else
            _curve->ContinueExistingArc(index1,BiquadraticCompositeCurve3::LEFT, col);

        _side_widget->deleteArcBox->addItem(colorName);
        _side_widget->selectArc1->addItem(colorName);
        _side_widget->selectArc2->addItem(colorName);

        break;
    }
    // Join
    case 3: {
        QString colorName = _side_widget->arcColorBox->currentText();
        Color4 *col = getColorFromString(colorName.toStdString());

        if(direction1 == "Right" && direction2 == "Left" )
              _curve->JoinExistingArcs(index1,BiquadraticCompositeCurve3::RIGHT,index2,BiquadraticCompositeCurve3::LEFT, col);
        else if(direction1 == "Right" && direction2 == "Right")
              _curve->JoinExistingArcs(index1,BiquadraticCompositeCurve3::RIGHT,index2,BiquadraticCompositeCurve3::RIGHT, col);
        else if(direction1 == "Left" && direction2 == "Right")
              _curve->JoinExistingArcs(index1,BiquadraticCompositeCurve3::LEFT,index2,BiquadraticCompositeCurve3::RIGHT, col);
        else if(direction1 == "Left" && direction2 == "Left")
              _curve->JoinExistingArcs(index1,BiquadraticCompositeCurve3::LEFT,index2,BiquadraticCompositeCurve3::LEFT, col);

        _side_widget->deleteArcBox->addItem(colorName);
        _side_widget->selectArc1->addItem(colorName);
        _side_widget->selectArc2->addItem(colorName);

        break;
    }
    // Merge
    case 4:
       if(direction1 == "Right" && direction2 == "Left" )
             _curve->MergeExistingArcs(index1,BiquadraticCompositeCurve3::RIGHT,index2,BiquadraticCompositeCurve3::LEFT);
       else if(direction1 == "Right" && direction2 == "Right")
             _curve->MergeExistingArcs(index1,BiquadraticCompositeCurve3::RIGHT,index2,BiquadraticCompositeCurve3::RIGHT);
       else if(direction1 == "Left" && direction2 == "Right")
             _curve->MergeExistingArcs(index1,BiquadraticCompositeCurve3::LEFT,index2,BiquadraticCompositeCurve3::RIGHT);
       else if(direction1 == "Left" && direction2 == "Left")
             _curve->MergeExistingArcs(index1,BiquadraticCompositeCurve3::LEFT,index2,BiquadraticCompositeCurve3::LEFT);
        break;
    }

    updateGL();
}

void GLWidget::change_arc_color(int value)
{
    GLuint index = _side_widget->selectArc1->currentIndex();
    switch (value) {
        case 0:
            _curve->changeArcColorByIndex(index,_colors[0]);
            _side_widget->deleteArcBox->removeItem(index);
            _side_widget->deleteArcBox->insertItem(index,"Red");

            _side_widget->selectArc1->removeItem(index);
            _side_widget->selectArc1->insertItem(index,"Red");

            _side_widget->selectArc2->removeItem(index);
            _side_widget->selectArc2->insertItem(index,"Red");

        break;
        case 1:
            _curve->changeArcColorByIndex(index,_colors[1]);
            _side_widget->deleteArcBox->removeItem(index);
            _side_widget->deleteArcBox->insertItem(index,"Green");

            _side_widget->selectArc1->removeItem(index);
            _side_widget->selectArc1->insertItem(index,"Green");

            _side_widget->selectArc2->removeItem(index);
            _side_widget->selectArc2->insertItem(index,"Green");
        break;
        case 2:
            _curve->changeArcColorByIndex(index,_colors[2]);

            _side_widget->deleteArcBox->removeItem(index);
            _side_widget->deleteArcBox->insertItem(index,"Blue");

            _side_widget->selectArc1->removeItem(index);
            _side_widget->selectArc1->insertItem(index,"Blue");

            _side_widget->selectArc2->removeItem(index);
            _side_widget->selectArc2->insertItem(index,"Blue");
        break;
        case 3:
            _curve->changeArcColorByIndex(index,_colors[3]);

            _side_widget->deleteArcBox->removeItem(index);
            _side_widget->deleteArcBox->insertItem(index,"Yellow");

            _side_widget->selectArc1->removeItem(index);
            _side_widget->selectArc1->insertItem(index,"Yellow");

            _side_widget->selectArc2->removeItem(index);
            _side_widget->selectArc2->insertItem(index,"Yellow");
        break;
        case 4:
            _curve->changeArcColorByIndex(index,_colors[4]);

            _side_widget->deleteArcBox->removeItem(index);
            _side_widget->deleteArcBox->insertItem(index,"Magenta");

            _side_widget->selectArc1->removeItem(index);
            _side_widget->selectArc1->insertItem(index,"Magenta");

            _side_widget->selectArc2->removeItem(index);
            _side_widget->selectArc2->insertItem(index,"Magenta");
        break;
        case 5:
            _curve->changeArcColorByIndex(index,_colors[5]);

            _side_widget->deleteArcBox->removeItem(index);
            _side_widget->deleteArcBox->insertItem(index,"Cyan");

            _side_widget->selectArc1->removeItem(index);
            _side_widget->selectArc1->insertItem(index,"Cyan");

            _side_widget->selectArc2->removeItem(index);
            _side_widget->selectArc2->insertItem(index,"Cyan");
        break;
        default:
            _curve->changeArcColorByIndex(index,_colors[0]);
            _side_widget->deleteArcBox->removeItem(index);
            _side_widget->deleteArcBox->insertItem(index,"Red");

            _side_widget->selectArc1->removeItem(index);
            _side_widget->selectArc1->insertItem(index,"Red");

            _side_widget->selectArc2->removeItem(index);
            _side_widget->selectArc2->insertItem(index,"Red");
        break;

    }

    updateGL();
}

GLuint GLWidget::get_index_by_color(Color4* color)
{
    std::vector<BiquadraticCompositeCurve3::ArcAttributes> attributes = _curve->get_attributes();
    for(GLuint i = 0; i < attributes.size(); i++)
    {
        cout<<i<<endl;
        if(attributes[i].color == color)
            return attributes[i].index;
    }

    return -1;
}

/* Patch methods */
void GLWidget::add_patch() {
    _side_widget->deletePatchBox->addItem(_side_widget->patchColorBox->currentText());
    _side_widget->selectPatch1->addItem(_side_widget->patchColorBox->currentText());
    _side_widget->selectPatch2->addItem(_side_widget->patchColorBox->currentText());

    switch (_side_widget->patchColorBox->currentIndex()) {
        case 0 :
            _composite_surface->InsertNewIsolatedPatch(_surfaceindex,MatFBRuby);
        break;
        case 1 :
            _composite_surface->InsertNewIsolatedPatch(_surfaceindex,MatFBEmerald);
        break;
        case 2:
            _composite_surface->InsertNewIsolatedPatch(_surfaceindex,MatFBTurquoise);
        break;
        case 3:
            _composite_surface->InsertNewIsolatedPatch(_surfaceindex,MatFBGold);
        break;
        case 4:
            _composite_surface->InsertNewIsolatedPatch(_surfaceindex,MatFBPearl);
        break;
        case 5:
            _composite_surface->InsertNewIsolatedPatch(_surfaceindex,MatFBBrass);
        break;
        case 6:
            _composite_surface->InsertNewIsolatedPatch(_surfaceindex,MatFBSilver);
        break;
    }


    _surfaceindex++;

    updateGL();
}

void GLWidget::delete_patch() {
   // _side_widget->selectPatch1->removeItem(_side_widget->deletePatchBox->currentIndex());
   // _side_widget->selectPatch2->removeItem(_side_widget->deletePatchBox->currentIndex());
   // _side_widget->deletePatchBox->removeItem(_side_widget->deletePatchBox->currentIndex());

    _side_widget->selectPatch1->clear();
    _side_widget->selectPatch2->clear();
    _side_widget->deletePatchBox->clear();

    _composite_surface->clear();
    _surfaceindex = 0;
    updateGL();
}

void GLWidget::do_patch_operation() {
    // If patch ID is string (else, get index)
    string patch1 = _side_widget->selectPatch1->currentText().toStdString();
    string patch2 = _side_widget->selectPatch2->currentText().toStdString();
    string direction1 = _side_widget->selectDirection1->currentText().toStdString();
    string direction2 = _side_widget->selectDirection2->currentText().toStdString();
    BiquadraticCompositeSurface3::Direction dir1, dir2;

    GLuint index1 = _side_widget->selectPatch1->currentIndex();
    GLuint index2 = _side_widget->selectPatch2->currentIndex();

    if(direction1 == "North")
        dir1 = BiquadraticCompositeSurface3::N;
    else if(direction1 == "West")
        dir1 = BiquadraticCompositeSurface3::W;
    else if(direction1 == "East")
        dir1 = BiquadraticCompositeSurface3::E;
    else if(direction1 == "South")
        dir1 = BiquadraticCompositeSurface3::S;
    else if(direction1 == "Northwest")
        dir1 = BiquadraticCompositeSurface3::NW;
    else if(direction1 == "Northeast")
        dir1 = BiquadraticCompositeSurface3::NE;
    else if(direction1 == "Southeast")
        dir1 = BiquadraticCompositeSurface3::SE;
    else if(direction1 == "Southwest")
        dir1 = BiquadraticCompositeSurface3::SW;

    if(direction2 == "North")
        dir2 = BiquadraticCompositeSurface3::N;
    else if(direction2 == "West")
        dir2 = BiquadraticCompositeSurface3::W;
    else if(direction2 == "East")
        dir2 = BiquadraticCompositeSurface3::E;
    else if(direction2 == "South")
        dir2 = BiquadraticCompositeSurface3::S;
    else if(direction2 == "Northwest")
        dir2 = BiquadraticCompositeSurface3::NW;
    else if(direction2 == "Northeast")
        dir2 = BiquadraticCompositeSurface3::NE;
    else if(direction2 == "Southeast")
        dir2 = BiquadraticCompositeSurface3::SE;
    else if(direction2 == "Southwest")
        dir2 = BiquadraticCompositeSurface3::SW;


    GLuint operation = _side_widget->patchOperation->currentIndex();

    switch (operation) {
    // Move
    case 0: {
        GLuint index = _side_widget->selectPatch1->currentIndex();
        //DCoordinate3 point;

        //if(!_composite_surface->getPatchAttributes(index).patch->GetData(_side_widget->selectRow->currentIndex(), _side_widget->selectColumn->currentIndex(), point))
         //   cout << "No data for patch" << endl;

        //GLuint xlocal = point.x(), ylocal = point.y();

        _composite_surface->MoveControlPoint(index, _side_widget->selectRow->currentIndex(), _side_widget->selectColumn->currentIndex(), _side_widget->xspinbox->value(), _side_widget->yspinbox->value(), _side_widget->zspinbox->value());
        break;
    }
    //Shift
    case 1:
        shift();
        break;

    // Continue
    case 2: {
        cout<<"Continue"<<endl;
        QString matName = _side_widget->patchColorBox->currentText();
        _composite_surface->ContinueExistingPatch(_composite_surface->getPatchIndex(index1), dir1, matName.toStdString());

        _side_widget->deletePatchBox->addItem(matName);
        _side_widget->selectPatch1->addItem(matName);
        _side_widget->selectPatch2->addItem(matName);
        break;
    }
    // Join
    case 3: {
        cout<<"Join"<<endl;
        QString matName = _side_widget->patchColorBox->currentText();
        _composite_surface->JoinExistingPatches(_composite_surface->getPatchIndex(index1),dir1,_composite_surface->getPatchIndex(index2),dir2,matName.toStdString());

        _side_widget->deletePatchBox->addItem(matName);
        _side_widget->selectPatch1->addItem(matName);
        _side_widget->selectPatch2->addItem(matName);
        break;
    }
    // Merge
    case 4:
        // merge(patch1, patch2, direction1, direction2);
        BiquadraticCompositeSurface3::Direction dir1,dir2;
        if(direction1 == "North")
        {
            dir1 = BiquadraticCompositeSurface3::N;
        }
        else if(direction1 == "East")
        {
            dir1 = BiquadraticCompositeSurface3::E;
        }
        else if(direction1 == "South")
        {
             dir1 = BiquadraticCompositeSurface3::S;
        }
        else if(direction1 == "West")
        {
            dir1 = BiquadraticCompositeSurface3::W;
        }
        else
        {
            cout<<"NORTHEAST FOR 1"<<endl;
            dir1 = BiquadraticCompositeSurface3::NE;
        }

        if(direction2 == "North")
        {
            dir2 = BiquadraticCompositeSurface3::N;
        }
        else if(direction2 == "East")
        {
            dir2 = BiquadraticCompositeSurface3::E;
        }
        else if(direction2 == "South")
        {
             dir2 = BiquadraticCompositeSurface3::S;
        }
        else if(direction2 == "West")
        {
            dir2 = BiquadraticCompositeSurface3::W;
        }
        else
        {
            cout<<"NORTHWEST FOR 2"<<endl;
            dir2 = BiquadraticCompositeSurface3::NW;
        }

        _composite_surface->MergeExistingPatches(_composite_surface->getPatchIndex(index1),dir1,_composite_surface->getPatchIndex(index2),dir2);
        break;
    }

    updateGL();
}

void GLWidget::shift()
{
    GLdouble xvalue = _side_widget->xspinbox->value();
    GLdouble yvalue = _side_widget->yspinbox->value();
    GLdouble zvalue = _side_widget->zspinbox->value();

    GLuint index = _side_widget->selectPatch1->currentIndex();

    _composite_surface->ShiftPatch(index,xvalue,yvalue,zvalue);

    updateGL();
}

void GLWidget::set_individual_shader(int value)
{
    GLuint index = _side_widget->selectPatch1->currentIndex();
    if(value != _shaderID)
    {
        _shaderID = value;
        _currentShader = _shaders(value);
        //_currentShader.Enable();
        //_composite_surface->SetShaderForAll(_currentShader);
        _composite_surface->SetShaderByIndex(index, _currentShader);
        updateGL();

        //_currentShader.Disable();
    }
}

void GLWidget::set_light(int v) {
    (v > 0) ? isLight = true : isLight = false;
    cout << "Light is " << isLight << endl;
    updateGL();
}

void GLWidget::change_patch_material(int value)
{
    GLuint patch = _side_widget->selectPatch1->currentIndex();

    switch (value) {
        case 0:
            _composite_surface->SetMaterialByIndex(patch,MatFBBrass);
            _side_widget->selectPatch1->removeItem(patch);
            _side_widget->selectPatch1->insertItem(patch,"Brass");

                _side_widget->deletePatchBox->removeItem(patch);
                _side_widget->deletePatchBox->insertItem(patch,"Brass");

                _side_widget->selectPatch2->removeItem(patch);
                _side_widget->selectPatch2->insertItem(patch,"Brass");


        break;
        case 1:
        _composite_surface->SetMaterialByIndex(patch,MatFBGold);
        _side_widget->selectPatch1->removeItem(patch);
        _side_widget->selectPatch1->insertItem(patch,"Gold");
        _side_widget->selectPatch1->currentIndexChanged(patch);

            _side_widget->deletePatchBox->removeItem(patch);
            _side_widget->deletePatchBox->insertItem(patch,"Gold");

            _side_widget->selectPatch2->removeItem(patch);
            _side_widget->selectPatch2->insertItem(patch,"Gold");
            _side_widget->selectPatch2->currentIndexChanged(patch);

        break;
        case 2:
        _composite_surface->SetMaterialByIndex(patch,MatFBSilver);
        _side_widget->selectPatch1->removeItem(patch);
        _side_widget->selectPatch1->insertItem(patch,"Silver");
        if(_side_widget->deletePatchBox->currentIndex() == patch)

            _side_widget->deletePatchBox->removeItem(patch);
            _side_widget->deletePatchBox->insertItem(patch,"Silver");


            _side_widget->selectPatch2->removeItem(patch);
            _side_widget->selectPatch2->insertItem(patch,"Silver");

        break;
        case 3:
        _composite_surface->SetMaterialByIndex(patch,MatFBEmerald);
        _side_widget->selectPatch1->removeItem(patch);
        _side_widget->selectPatch1->insertItem(patch,"Emerald");

            _side_widget->deletePatchBox->removeItem(patch);
            _side_widget->deletePatchBox->insertItem(patch,"Emerald");

            _side_widget->selectPatch2->removeItem(patch);
            _side_widget->selectPatch2->insertItem(patch,"Emerald");

        break;
        case 4:
        _composite_surface->SetMaterialByIndex(patch,MatFBPearl);
        _side_widget->selectPatch1->removeItem(patch);
        _side_widget->selectPatch1->insertItem(patch,"Pearl");

            _side_widget->deletePatchBox->removeItem(patch);
            _side_widget->deletePatchBox->insertItem(patch,"Pearl");

            _side_widget->selectPatch2->removeItem(patch);
            _side_widget->selectPatch2->insertItem(patch,"Pearl");

        break;
        case 5:
        _composite_surface->SetMaterialByIndex(patch,MatFBRuby);
        _side_widget->selectPatch1->removeItem(patch);
        _side_widget->selectPatch1->insertItem(patch,"Ruby");

            _side_widget->deletePatchBox->removeItem(patch);
            _side_widget->deletePatchBox->insertItem(patch,"Ruby");

            _side_widget->selectPatch2->removeItem(patch);
            _side_widget->selectPatch2->insertItem(patch,"Ruby");

        break;
        case 6:
        _composite_surface->SetMaterialByIndex(patch,MatFBTurquoise);
        _side_widget->selectPatch1->removeItem(patch);
        _side_widget->selectPatch1->insertItem(patch,"Turqoise");
        _side_widget->deletePatchBox->removeItem(patch);
        _side_widget->deletePatchBox->insertItem(patch,"Turqoise");
        _side_widget->selectPatch2->removeItem(patch);
        _side_widget->selectPatch2->insertItem(patch,"Turqoise");

        break;
    }

    updateGL();
}
/* Curve */
void GLWidget::curve()
{
    _curve = new BiquadraticCompositeCurve3();
}

void GLWidget::loadColors()
{
    //red
    _colors[0] = new Color4(1.0,0.0,0.0,1.0);
    //green
    _colors[1] = new Color4(0.0,1.0,0.0,1.0);
    //blue
    _colors[2] = new Color4(0.0,0.0,1.0,1.0);
    //yellow
    _colors[3] = new Color4(1.0,1.0,0.0,1.0);
    //magenta
    _colors[4] = new Color4(1.0,0.0,1.0,1.0);
    //cyan
    _colors[5] = new Color4(0.0,1.0,1.0,1.0);
}

void GLWidget::surface()
{
    _composite_surface = new BiquadraticCompositeSurface3();
}

void GLWidget::loadFromFile()
{
    string path;
    string s = _side_widget->lineEdit->text().toStdString();
    if(_homeworkID == 2) // curve
    {
        path = "Curves/";
        GLuint added = 0;
        added =_curve->ReadCurveFromFile(path+s,_curveindex);
        GLuint j = 0;

        for(GLuint i = _curveindex ; i < added + _curveindex; i++)
        {

            Color4* color = getColorFromString(_curve->loadedColors[j]);
            _curve->changeArcColorByIndex(i,color);
            string c = _curve->loadedColors[j];
            QString x = QString::fromStdString(c);
            _side_widget->deleteArcBox->addItem(x);
            _side_widget->selectArc1->addItem(x);
            _side_widget->selectArc2->addItem(x);
            j++;
        }

        _curveindex += added;



    }
    else if(_homeworkID == 3) // surface
    {
         path = "Surfaces/";
         GLuint added = 0;
         added = _composite_surface->ReadSurfaceFromFile(path+s,_surfaceindex);
         cout<<added<<endl;


         GLuint j = 0;
         for(GLuint i = _surfaceindex; i < added + _surfaceindex; i++)
         {
            string mat = getMaterialName( _composite_surface->loaded_materials[j]);
            QString c = QString::fromStdString(mat);
            _side_widget->deletePatchBox->addItem(c);
            _side_widget->selectPatch1->addItem(c);
            _side_widget->selectPatch2->addItem(c);
            j++;
         }

         _surfaceindex += added;
    }
    updateGL();

}

void GLWidget::saveToFile()
{
    string path;
    string s = _side_widget->lineEdit->text().toStdString();
    if(_homeworkID == 2) // curve
    {
        path = "Curves/";

        _curve->SaveCurveToFile(path+s, _colors);
    }
    else if(_homeworkID == 3) // surface
    {
         path = "Surfaces/";
         _composite_surface->SaveSurfaceToFile(path+s);
    }

    updateGL();
}

Color4* GLWidget::getColorFromString(std::string c)
{
    if(c == "Red")
        return _colors[0];
    if(c == "Green")
        return _colors[1];
    if(c == "Blue")
        return _colors[2];
    if(c == "Yellow")
        return _colors[3];
    if(c == "Magenta")
        return _colors[4];
    if(c == "Cyan")
        return _colors[5];

    return _colors[0];
}

std::string GLWidget::getMaterialName(Material *m)
{
    if(m == &MatFBGold)
        return "Gold";
    if(m == &MatFBRuby)
        return "Ruby";
    if(m == &MatFBBrass)
        return "Brass";
    if(m == &MatFBPearl)
        return "Pearl";
    if(m == &MatFBSilver)
        return "Silver";
    if(m == &MatFBEmerald)
        return "Emerald";
    if(m == &MatFBTurquoise)
        return "Turqoise";
    return "Ruby";
}

GLWidget::~GLWidget()
{
    if (_pc) delete _pc;
    _pc = nullptr;

    if (_image_of_pc) delete _image_of_pc;
    _image_of_pc = nullptr;

    if(_parametricsurface)
        delete _parametricsurface,_parametricsurface = nullptr;
    if(_surface)
        delete _surface,_surface = nullptr;

    if(_before_interpolation)
    {
        delete _before_interpolation, _before_interpolation = 0;
    }

    if(_after_interpolation)
    {
        delete _after_interpolation, _after_interpolation = 0;
    }
    if(_arc)
    {
        delete _arc,_arc = 0;
    }
    if (_image_of_arc) delete _image_of_arc;
    _image_of_arc = nullptr;

    if(_curve)
        delete _curve, _curve = nullptr;

    if(_composite_surface)
        delete _composite_surface,_composite_surface = nullptr;
}
}
