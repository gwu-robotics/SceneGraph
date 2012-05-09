#ifndef _GL_MESH_H_
#define _GL_MESH_H_

#include <SimpleGui/GLObject.h>
#include <SimpleGui/GLCVars.h>

#include <assimp/assimp.h>
#include <assimp/aiPostProcess.h>
#include <assimp/aiScene.h>

class GLMesh : public GLObject
{

    public:
        ////////////////////////////////////////////////////////////////////////////
        GLMesh()
        {
            m_sObjectName = "Mesh";
            m_nDisplayList = -1;
            m_fScale = 1;
            m_fAlpha = 1;
            m_iMeshID = -1;
            m_bSelectionIDAllocated = false;
            m_flScale = 1.0f;
        }

        ////////////////////////////////////////////////////////////////////////////
        void Init( const std::string& sMeshFile )
        {
            m_pScene = aiImportFile( sMeshFile.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals );
            if( m_pScene == NULL ){
                printf("ERROR: loading mesh '%s'\n", sMeshFile.c_str() );
            }
        }

        ////////////////////////////////////////////////////////////////////////////
        void Init( const struct aiScene* pScene )
        {
            m_pScene = pScene;
        }

        void AllocateSelectionID()
        {
            if ( m_bSelectionIDAllocated )
                return; // selection ID has already been allocated; abort

            m_iMeshID = AllocSelectionId();

            m_bSelectionIDAllocated = true;
        }

        ////////////////////////////////////////////////////////////////////////////
        void color4_to_float4( const struct aiColor4D *c, float f[4] )
        {
            f[0] = c->r;
            f[1] = c->g;
            f[2] = c->b;
            f[3] = c->a;
        }

        ////////////////////////////////////////////////////////////////////////////
        void set_float4(float f[4], float a, float b, float c, float d)
        {
            f[0] = a;
            f[1] = b;
            f[2] = c;
            f[3] = d;
        }


        ////////////////////////////////////////////////////////////////////////////
        void ApplyMaterial( const struct aiMaterial *mtl )
        {

            //            glShadeModel( GL_FLAT );
            glDisable( GL_COLOR_MATERIAL );                                 // activate material

            float c[4];

            GLenum fill_mode;
            int ret1, ret2;
            struct aiColor4D diffuse;
            struct aiColor4D specular;
            struct aiColor4D ambient;
            struct aiColor4D emission;
            float shininess, strength;
            int two_sided;
            int wireframe;
            unsigned int max;

            set_float4( c, 0.8f, 0.8f, 0.8f, 1.0f );
            if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse)){
                color4_to_float4( &diffuse, c);
                //    printf("Applying GL_DIFFUSE %f, %f, %f, %f\n", c[0], c[1], c[2], c[3] );
            }
            glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, c );

            set_float4( c, 0.0f, 0.0f, 0.0f, 1.0f );
            if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular)){
                color4_to_float4(&specular, c);
                //    printf("Applying GL_SPECULAR %f, %f, %f, %f\n", c[0], c[1], c[2], c[3] );
            }
            glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, c );

            set_float4( c, 0.2f, 0.2f, 0.2f, 1.0f );
            if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient)){
                color4_to_float4( &ambient, c );
                //   printf("Applying GL_AMBIENT %f, %f, %f, %f\n", c[0], c[1], c[2], c[3] );
            }
            glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, c );

            set_float4( c, 0.0f, 0.0f, 0.0f, 1.0f );
            if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission)){
                color4_to_float4(&emission, c);
                //    printf("Applying GL_EMISSION %f, %f, %f, %f\n", c[0], c[1], c[2], c[3] );
            }
            glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, c );


            CheckForGLErrors();


            max = 1;
            ret1 = aiGetMaterialFloatArray( mtl, AI_MATKEY_SHININESS, &shininess, &max );
            if( ret1 == AI_SUCCESS ){
                max = 1;
                ret2 = aiGetMaterialFloatArray( mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max );
                if(ret2 == AI_SUCCESS){
                    glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength );
                    CheckForGLErrors();
                }
                else{
                    //                    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
                    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 1 );
                    CheckForGLErrors();
                }
            }
            else {
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
                set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
                CheckForGLErrors();
            }

            return;
            max = 1;
            if( AI_SUCCESS == aiGetMaterialIntegerArray(
                        mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max) ){
                fill_mode = wireframe ? GL_LINE : GL_FILL;
            }
            else{
                fill_mode = GL_FILL;
            }
            glPolygonMode( GL_FRONT_AND_BACK, fill_mode );
            CheckForGLErrors();

            max = 1;
            if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided){
                glDisable(GL_CULL_FACE);
                CheckForGLErrors();
            }
            else{
                glEnable(GL_CULL_FACE);
                CheckForGLErrors();
            }

        }

        ////////////////////////////////////////////////////////////////////////////
        void RenderFace( GLenum face_mode, GLenum fill_mode, float fAlpha, const struct aiFace* face,  const struct aiMesh* mesh )
        {
            glPolygonMode( GL_FRONT_AND_BACK, fill_mode );
            glBegin( face_mode );
            for( unsigned int ii = 0; ii < face->mNumIndices; ii++ ) {
                int index = face->mIndices[ii];
                if( mesh->mColors[0] != NULL ){
                    float *c = (float*)&mesh->mColors[0][index];
                    glColor4f( c[0], c[1], c[2], fAlpha*c[3] );
                }
                if( mesh->mNormals != NULL ){
                    glNormal3fv( &mesh->mNormals[index].x );
                    //       printf( "Normal %f, %f, %f\n", mesh->mNormals[index].x,
                    //            mesh->mNormals[index].y, mesh->mNormals[index].z );
                    //                    glNormal3f( -mesh->mNormals[index].x, -mesh->mNormals[index].y, -mesh->mNormals[index].z );
                }
                glVertex3fv( &mesh->mVertices[index].x );
            }
            glEnd();

            // show normals for debugging
            if( gConfig.m_bShowMeshNormals ){
                float s = 10;
                glBegin( GL_LINES );
                for( unsigned int ii = 0; ii < face->mNumIndices; ii++ ) {
                    int index = face->mIndices[ii];
                    float* p = &mesh->mVertices[index].x;
                    float* n = &mesh->mNormals[index].x;
                    glVertex3f( p[0], p[1], p[2] );
                    glVertex3f( p[0]+s*n[0], p[1]+s*n[1], p[2]+s*n[2] );
                }
                glEnd();
            }

        }

        ////////////////////////////////////////////////////////////////////////////
        void RenderMesh(
                const struct aiMesh* mesh,
                const struct aiMaterial* mtl
                )
        {
            ApplyMaterial( mtl );
            if( mesh->mNormals == NULL ) {
                glDisable(GL_LIGHTING);
            } else {
                glEnable(GL_LIGHTING);
            }

            for( unsigned int t = 0; t < mesh->mNumFaces; ++t) {
                const struct aiFace* face = &mesh->mFaces[t];
                GLenum face_mode;

                switch(face->mNumIndices) {
                    case 1: face_mode = GL_POINTS; break;
                    case 2: face_mode = GL_LINES; break;
                    case 3: face_mode = GL_TRIANGLES; break;
                    default: face_mode = GL_POLYGON; break;
                }
                RenderFace( face_mode, GL_FILL, m_fAlpha, face, mesh );
            }
        }

        ////////////////////////////////////////////////////////////////////////////
        void RecursiveRender( const struct aiScene *sc, const struct aiNode* nd )
        {
            unsigned int n = 0;
            struct aiMatrix4x4 m = nd->mTransformation;

            // update transform
            aiTransposeMatrix4( &m );
            glPushMatrix();
            glMultMatrixf( (float*)&m );

            // draw all meshes assigned to this node
            for (; n < nd->mNumMeshes; ++n) {
                const struct aiMesh* mesh = m_pScene->mMeshes[nd->mMeshes[n]];
                RenderMesh( mesh, sc->mMaterials[mesh->mMaterialIndex] );

                /*
                //                glDisable(GL_LIGHTING);
                //                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                //                glEnable (GL_BLEND );
                for (t = 0; t < mesh->mNumFaces; ++t) {
                const struct aiFace* face = &mesh->mFaces[t];
                GLenum face_mode;
                switch(face->mNumIndices) {
                case 1: face_mode = GL_POINTS; break;
                case 2: face_mode = GL_LINES; break;
                case 3: face_mode = GL_TRIANGLES; break;
                default: face_mode = GL_POLYGON; break;
                }
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
                glBegin( face_mode );
                for( int i = 0; i < face->mNumIndices; i++) {
                int index = face->mIndices[i];
                //glColor4f( 1.0, 1.0, 1.0, 0.2 );
                glVertex3fv(&mesh->mVertices[index].x);
                }
                glEnd();

                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
                glBegin( face_mode );
                for( int i = 0; i < face->mNumIndices; i++) {
                int index = face->mIndices[i];
                glColor4f( 1.0, 1.0, 1.0, 1 );
                glVertex3fv(&mesh->mVertices[index].x);
                }
                glEnd();
                }
                 */
            }


            // draw all children
            for( n = 0; n < nd->mNumChildren; ++n ) {
                RecursiveRender( sc, nd->mChildren[n] );
            }

            glPopMatrix();
        }

        ////////////////////////////////////////////////////////////////////////////
        virtual void  draw()
        {
            if( m_pScene ){
                glEnable( GL_DEPTH_TEST );
                if( m_nDisplayList == -1 ){
                    m_nDisplayList = glGenLists(1);
                    glNewList( m_nDisplayList, GL_COMPILE );
                    RecursiveRender( m_pScene, m_pScene->mRootNode );
                    glEndList();
                }

                //               AllocateSelectionID(); holy cow this is badness batman.  should only call this guy from init code... GTS

                glPushMatrix();

                glTranslated( m_dPosition[0], m_dPosition[1], m_dPosition[2] );

                // TODO: Rotations over the world axis instead of local axis
                // Doing these rotations one after another results in incremental rotations whose results are dependent on each other
                // I spent several days trying various different methods to come up with a solution, but eventually I just gave up
                // I am hoping to tackle this again sometime, but I need a break from it for now...
                glRotated( m_dPosition[3], 1.0f, 0.0f, 0.0f );
                glRotated( m_dPosition[4], 0.0f, 1.0f, 0.0f );
                glRotated( m_dPosition[5], 0.0f, 0.0f, 1.0f );

                glScalef( m_fScale, m_fScale, m_fScale );

                glPushName( m_iMeshID );
                glCallList( m_nDisplayList );
                glPopName();

                glPopMatrix();
            }
        }

        ///////////////////////////
        virtual void ComputeDimensions()
        {
            Eigen::Vector3d min, max;
            aiMesh *pAIMesh;
            aiVector3D *pAIVector;

            for( unsigned int x = 0; x < this->GetScene()->mNumMeshes; x++ ){
                pAIMesh = this->GetScene()->mMeshes[x];
                if ( pAIMesh == NULL )
                    continue;

                for( unsigned int y = 0; y < pAIMesh->mNumVertices; y++ ){
                    pAIVector = &pAIMesh->mVertices[y];
                    if ( pAIVector == NULL ){
                        continue;
                        }

                    if ( ((pAIVector->x * this->GetScale()) < min[0]) && ((pAIVector->y * this->GetScale()) < min[1]) && ((pAIVector->z * this->GetScale()) < min[2]) ){
                        min[0] = pAIVector->x * this->GetScale();
                        min[1] = pAIVector->y * this->GetScale();
                        min[2] = pAIVector->z * this->GetScale();
                    }

                    if ( ((pAIVector->x * this->GetScale()) > max[0]) && ((pAIVector->y * this->GetScale()) > max[1]) && ((pAIVector->z * this->GetScale()) > max[2]) ) {
                        max[0] = pAIVector->x * this->GetScale();
                        max[1] = pAIVector->y * this->GetScale();
                        max[2] = pAIVector->z * this->GetScale();
                    }
                }
            }

            m_Dimensions[0] = max[0] - min[0];
            m_Dimensions[1] = max[1] - min[1];
            m_Dimensions[2] = max[2] - min[2];
        }

        // Getters and setters
        const struct aiScene *GetScene( void ) { return m_pScene; }

        virtual void select( unsigned int )
        {
            // WARNING: When an instance of GLMesh is selected, it appears that it remains selected forever
            // One way to resolve this is to call 'UnSelect( m_iMeshID )' after doing anything pertaining to selection
            // Hopefully we find a better, more permanent solution soon...

            // UPDATE: Don't know if what's above is still a valid statement...
        }

        Eigen::Vector3d GetDimensions() { return m_Dimensions; }

        float GetScale() { return m_flScale; }
        void SetScale( float flScale ) { m_flScale = flScale; }

    protected:
        const struct aiScene*   m_pScene;
        float                   m_fScale;
        float                   m_fAlpha; // render translucent meshes?
        GLint                   m_nDisplayList;
        unsigned int            m_iMeshID;
        bool                    m_bSelectionIDAllocated;
        Eigen::Vector3d         m_Dimensions;
        float                   m_flScale;
};


#endif

