/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include <gtest/gtest.h>

#include "OgreRoot.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"
#include "OgreCamera.h"
#include "RootWithoutRenderSystemFixture.h"

using namespace Ogre;

TEST(Root,shutdown)
{
    Root root;
    root.shutdown();
}

TEST(SceneManager,removeAndDestroyAllChildren)
{
    Root root;
    SceneManager* sm = root.createSceneManager();
    sm->getRootSceneNode()->createChildSceneNode();
    sm->getRootSceneNode()->createChildSceneNode();
    sm->getRootSceneNode()->removeAndDestroyAllChildren();
}

static void createRandomEntityClones(Entity* ent, size_t cloneCount, const Vector3& min,
                                     const Vector3& max, SceneManager* mgr)
{
    // Seed rand with a predictable value
    srand(5); // 5 is completely arbitrary, the idea is simply to use a constant
    for (size_t n = 0; n < cloneCount; ++n)
    {
        // Create a new node under the root.
        SceneNode* node = mgr->createSceneNode();
        // Random translate.
        Vector3 nodePos;
        nodePos.x = Math::RangeRandom(min.x, max.x);
        nodePos.y = Math::RangeRandom(min.y, max.y);
        nodePos.z = Math::RangeRandom(min.z, max.z);
        node->setPosition(nodePos);
        mgr->getRootSceneNode()->addChild(node);
        Entity* cloneEnt = ent->clone(StringConverter::toString(n));
        // Attach to new node.
        node->attachObject(cloneEnt);
    }
}

struct SceneQueryTest : public RootWithoutRenderSystemFixture {
    SceneManager* mSceneMgr;
    Camera* mCamera;
    SceneNode* mCameraNode;

    void SetUp() {
        RootWithoutRenderSystemFixture::SetUp();

        mSceneMgr = mRoot->createSceneManager();
        mCamera = mSceneMgr->createCamera("Camera");
        mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mCameraNode->attachObject(mCamera);
        mCameraNode->setPosition(0,0,500);
        mCameraNode->lookAt(Vector3(0, 0, 0), Node::TS_PARENT);

        // Create a set of random balls
        Entity* ent = mSceneMgr->createEntity("501", "sphere.mesh", "General");

        // stick one at the origin so one will always be hit by ray
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
        createRandomEntityClones(ent, 500, Vector3(-2500,-2500,-2500), Vector3(2500,2500,2500), mSceneMgr);

        mSceneMgr->_updateSceneGraph(mCamera);
    }
};

TEST_F(SceneQueryTest,Intersection)
{
    IntersectionSceneQuery* intersectionQuery = mSceneMgr->createIntersectionQuery();

    int expected[][2] = {
        {1, 421},   {102, 356}, {108, 269}, {116, 239}, {118, 409}, {122, 60},  {125, 129},
        {127, 51},  {142, 175}, {144, 371}, {150, 501}, {152, 315}, {164, 4},   {185, 366},
        {190, 484}, {199, 448}, {205, 386}, {212, 60},  {224, 288}, {228, 287}, {232, 284},
        {24, 498},  {246, 280}, {254, 277}, {274, 406}, {286, 497}, {297, 444}, {313, 34},
        {313, 65},  {328, 336}, {348, 384}, {350, 466}, {358, 377}, {36, 39},   {360, 499},
        {365, 488}, {368, 63},  {372, 403}, {376, 458}, {382, 475}, {426, 487}, {450, 462},
        {456, 50},  {464, 77},  {480, 87}
    };

    IntersectionSceneQueryResult& results = intersectionQuery->execute();
    EXPECT_EQ(results.movables2movables.size(), sizeof(expected)/sizeof(expected[0]));

    int i = 0;
    for (SceneQueryMovableIntersectionList::iterator mov = results.movables2movables.begin();
         mov != results.movables2movables.end(); ++mov)
    {
        SceneQueryMovableObjectPair& thepair = *mov;
        EXPECT_EQ(expected[i][0], StringConverter::parseInt(thepair.first->getName()));
        EXPECT_EQ(expected[i][1], StringConverter::parseInt(thepair.second->getName()));
        i++;
    }
}

TEST_F(SceneQueryTest, Ray) {
    RaySceneQuery* rayQuery = mSceneMgr->createRayQuery(mCamera->getCameraToViewportRay(0.5, 0.5));
    rayQuery->setSortByDistance(true, 2);

    RaySceneQueryResult& results = rayQuery->execute();

    ASSERT_EQ("501", results[0].movable->getName());
    ASSERT_EQ("150", results[1].movable->getName());
}
