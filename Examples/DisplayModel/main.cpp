#include <iostream>
#include <boost/bind.hpp>
#include <Eigen/Eigen>

#include <pangolin/pangolin.h>
#include <SceneGraph/SceneGraph.h>

using namespace std;

void GlobalKeyHook(std::string str)
{
    cout << str << endl;
}

int main( int /*argc*/, char** /*argv[]*/ )
{
    // Create OpenGL window in single line thanks to GLUT
    pangolin::CreateGlutWindowAndBind("Main",640*2,480);
    SceneGraph::GLSceneGraph::ApplyPreferredGlSettings();
    glewInit();

    // Scenegraph to hold GLObjects and relative transformations
    SceneGraph::GLSceneGraph glGraph;

    SceneGraph::GLLight light(10,10,-100);
    glGraph.AddChild(&light);

    // Define grid object
    SceneGraph::GLGrid glGrid(50,2.0, true);

    // Define axis object, and set its pose
    SceneGraph::GLAxis glAxis;
    glAxis.SetPose(-1,-2,-0.1, 0, 0, M_PI/4);
    glAxis.SetScale(0.25);

    SceneGraph::GLMovableAxis glMovableAxis;
    glMovableAxis.SetPosition(-3,3,-1);

    SceneGraph::GLAxisAlignedBox glBox;
    glBox.SetResizable();

    // Define movable waypoint object with velocity
    SceneGraph::GLWayPoint glWaypoint;
    glWaypoint.SetPose(0.5,0.5,-0.1,0,0,0);

    // Optionally clamp waypoint to specific plane
    glWaypoint.ClampToPlane(Eigen::Vector4d(0,0,1,0));

    // Define 3D spiral using a GLCachedPrimitives object
    SceneGraph::GLCachedPrimitives glSpiral(GL_LINE_STRIP, SceneGraph::GLColor(1.0f,0.7f,0.2f));
    for(double t=0; t < 10*M_PI; t+= M_PI/50) {
        glSpiral.AddVertex(Eigen::Vector3d(cos(t)+2, sin(t)+2, -0.1*t) );
    }

    // Define 3D floating text object
    SceneGraph::GLText glText3d("3D Floating Text", -1, 1, -1);

#ifdef HAVE_ASSIMP
    // Define a mesh object and try to load model
    SceneGraph::GLMesh glMesh;
    try {
        glMesh.Init("./model.blend");
        glMesh.SetPosition(0,0,-0.15);
        glGraph.AddChild(&glMesh);
    }catch(exception e) {
        cerr << "Cannot load mesh. Check file exists" << endl;
    }
#endif

    // Add objects to scenegraph
    glGraph.AddChild(&glGrid);
    glGraph.AddChild(&glWaypoint);
    glGraph.AddChild(&glSpiral);
    glGraph.AddChild(&glAxis);
    glGraph.AddChild(&glText3d);
    glGraph.AddChild(&glMovableAxis);
    glMovableAxis.AddChild(&glBox);

    // We can have more than one scenegraph. This 2d scenegraph will
    // be rendered with an orthographic projection. This is useful
    // for text overlays.
    SceneGraph::GLSceneGraph glGraph2d;

    SceneGraph::GLText glText2d("2D Overlay Text", 10, 460);
    glGraph2d.AddChild(&glText2d);

    // Define Camera Render Object (for view / scene browsing)
    pangolin::OpenGlRenderState stacks3d(
        pangolin::ProjectionMatrix(640,480,420,420,320,240,0.1,1000),
        pangolin::ModelViewLookAt(0,-2,-4, 0,1,0, pangolin::AxisNegZ)
    );

    // Define second camera render object
    pangolin::OpenGlRenderState stacks3d_2(
        pangolin::ProjectionMatrix(640,480,420,420,320,240,0.1,1000),
        pangolin::ModelViewLookAt(2,-1,-2, 0,1,0, pangolin::AxisNegZ)
    );

    // Use different orthographic render state for 2D drawing.
    // Virtualise screen resolution to 640x480
    pangolin::OpenGlRenderState stacks2d(
        pangolin::ProjectionMatrixOrthographic(0,640,0,480,0,1000)
    );

    // Pangolin abstracts the OpenGL viewport as a View.
    // Here we get a reference to the default 'base' view.
    pangolin::View& container = pangolin::DisplayBase();

    // We define a new view which will reside within the container.
    pangolin::View view3d;

    // We set the views location on screen and add a handler which will
    // let user input update the model_view matrix (stacks3d) and feed through
    // to our scenegraph
    view3d.SetBounds(0.0, 1.0, 0.0, 1.0/2.0, 640.0f/480.0f)
          .SetHandler(new SceneGraph::HandlerSceneGraph(glGraph,stacks3d,pangolin::AxisNegZ))
          .SetDrawFunction(SceneGraph::ActivateDrawFunctor(glGraph, stacks3d));

    // We can define another view on the same scenegraph. We can also
    // render a second scenegraph as a 2D overlay using the different
    // 2D ModelView and Projection matrices (stacks3d)
    pangolin::View view3d_2d;
    view3d_2d.SetBounds(0.0,1.0, 1.0/2.0, 1.0, 640.0f/480.0f)
          .SetHandler(new SceneGraph::HandlerSceneGraph(glGraph,stacks3d_2,pangolin::AxisNegZ))
          .SetDrawFunction(SceneGraph::ActivateDrawFunctor3d2d(glGraph, stacks3d_2, glGraph2d, stacks2d));

    // Add our views as children to the base container.
    container.AddDisplay(view3d);
    container.AddDisplay(view3d_2d);

    // Demonstration of how we can register a keyboard hook to trigger a method
    pangolin::RegisterKeyPressCallback( pangolin::PANGO_CTRL + 'r', boost::bind(GlobalKeyHook, "You Pushed ctrl-r!" ) );

    // Add keyhook to save window contents (including alpha). The framebuffer is saved just before it is swapped
    pangolin::RegisterKeyPressCallback( 's', boost::bind(&pangolin::View::SaveOnRender, &pangolin::DisplayBase(), "window_OnRender" ) );

    // Add keyhook to save a particular view (including alpha) at 4 times the resolution of the screen. This creates an FBO and renders into it straight away.
    pangolin::RegisterKeyPressCallback( 'r', boost::bind(&pangolin::View::SaveRenderNow, &view3d, "view3d_RenderNow", 4 ) );

    // Default hooks for exiting (Esc) and fullscreen (tab).
    while( !pangolin::ShouldQuit() )
    {
        // Clear whole screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Swap frames and Process Events
        pangolin::FinishGlutFrame();

        // Pause for 1/60th of a second.
        usleep(1E6 / 60);
    }

    return 0;
}
