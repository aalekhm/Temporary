//---------------------------------------------------------------------------------------------
// Torque Game Builder
// Copyright (C) GarageGames.com, Inc.
//---------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------
// startGame
// All game logic should be set up here. This will be called by the level builder when you
// select "Run Game" or by the startup process of your game to load the first level.
//---------------------------------------------------------------------------------------------
function startGame(%level)
{
   exec("./Vectors.cs");
   exec("./VoronoiRegions.cs");
   exec("./CIRCLE_90.cs");
   
   Canvas.setContent(mainScreenGui);
   Canvas.setCursor(DefaultCursor);
   //Canvas.pushDialog(HUD);
   
   new ActionMap(moveMap);   
   moveMap.push();
   
   $enableDirectInput = true;
   activateDirectInput();
   enableJoystick();
   
   sceneWindow2D.loadLevel(%level);
   
   $line = point.clone(true);
   $line.setSize(1024, 1);
   $line.setBlendColor(0, 255, 255, 255);

   $axis = point.clone(true);
   $axis.setSize(1024, 1);
   $axis.setBlendColor(0, 255, 0, 255);
   
   $debug = !false;
   
   
   Poly_0.drawPolygonEdges();
   Poly_1.drawPolygonEdges();
   Poly_2.drawPolygonEdges();   
   Poly_3.drawPolygonEdges();

   //SAT - Separating Axis Theorem - 2 Polygons(AABB)
   checkAABBCollision(Poly_0, Poly_1);
   
   
   //SAT - Separating Axis Theorem with a Circle 
   checkAABBCircleCollision(Poly_2, CIRC_FULL);
   

   //SAT - Separating Axis Theorem with a Circle 
   checkAABBCircle90Collision(Poly_3, CIRC_90);

   //startSAT();
}

//---------------------------------------------------------------------------------------------
// endGame
// Game cleanup should be done here.
//---------------------------------------------------------------------------------------------
function endGame()
{
   sceneWindow2D.endLevel();
   moveMap.pop();
   moveMap.delete();
}  

function startSAT() {
   %angle = calculateAngle(ptA.getPositionX(), ptA.getPositionY(), ptB.getPositionX(), ptB.getPositionY());
   $line.setPosition(ptA.getPosition());
   $line.setRotation(%angle);
   
   %vectEdge = t2dVectorSub(ptA.getPosition(), ptB.getPosition());
   
   %pointB = -getWord(%vectEdge, 1) SPC getWord(%vectEdge, 0);//LeftHandNormal
   %pointC = getWord(%vectEdge, 1) SPC -getWord(%vectEdge, 0);//RightHandNormal
      
   $axis.setPosition(getWord(%pointB, 0), getWord(%pointB, 1));
   //$axis.setRotation(90+%angle);
   //$axis.setRotation(calculateAngle(getWord(%pointB, 0), getWord(%pointB, 1), 0, 0));
   $axis.setRotation(calculateAngle(getWord(%pointB, 0), getWord(%pointB, 1), getWord(%pointC, 0), getWord(%pointC, 1)));


   %edgeNormalized = t2dVectorNormalise(%vectEdge);
   echo("%vectEdge == "@%vectEdge@" , Normalized == "@%edgeNormalized@" "@t2dVectorLength(%vectEdge)@" , "@t2dVectorDistance(ptA.getPosition(), ptB.getPosition()));
   echo("%vectEdge == "@t2dVectorScale(%edgeNormalized, 100));
   %at100M = t2dVectorScale(%edgeNormalized, -100);
   %dot100 = point.clone(true);
   %dot100.setBlendColor(255, 0, 0, 255);
   %dot100.setPosition(t2dVectorAdd(%at100M, ptA.getPosition()));
/*
                                4.
                                 |
                                 |
                    (-1, 3).    3.                           
                                 |
                                 |
                                2.
                                 |
                                 |
                                1.                 .(3, 1)
                                 |     
                                 |
   <-----------------------.-----.-----.-----.-----.-------------------------->   
                           -1    0     1     2     3
   
   
*/   
   
}




function calculateAngle(%x1, %y1, %x2, %y2)
{
   %angleInRadians = mAtan(%y2 - %y1, %x2 - %x1);
   %angleInDegrees = mRadToDeg(%angleInRadians);

   return %angleInDegrees;
}

function MovablePoint::onMouseDragged(%this, %modifier, %worldPosition, %clicks) {
   %this.setPosition(%worldPosition);
   //startSAT();
}

function sceneWindow2D::onMouseDown(%this, %modifier, %worldPosition, %clicks) {
         %sg = %this.getSceneGraph();
         %this.DRAG_OBJ = getWord(%sg.pickPoint(%worldPosition), 0);
}

function MovePoly::onMouseDown(%this, %modifier, %worldPosition, %clicks) {
   if(sceneWindow2D.DRAG_OBJ == %this) {
      %this.mousePt = %worldPosition;
   }
}

function MovePoly::onMouseDragged(%this, %modifier, %worldPosition, %clicks) {
   if(sceneWindow2D.DRAG_OBJ == %this) {
      %delta = t2dVectorSub(%worldPosition, %this.mousePt);
      %newPos = t2dVectorAdd(%this.getPosition(), %delta);
      %this.setPosition(%newPos);
      
      if(%this.getName() $= "Poly_0" || %this.getName() $= "Poly_1")
         checkAABBCollision(Poly_0, Poly_1);
      else
      if(%this.getName() $= "Poly_2")
         checkAABBCircleCollision(Poly_2, CIRC_FULL);
      else
      if(%this.getName() $= "Poly_3")
         checkAABBCircle90Collision(Poly_3, CIRC_90);
      
      %this.mousePt = %worldPosition;
   }
}

function VERTICES::onMouseDragged(%this, %modifier, %worldPosition, %clicks) {
   if(sceneWindow2D.DRAG_OBJ == %this) {
      %parent = %this.getMountedParent();
      %this.dismount();
      
      %this.setPosition(%worldPosition);
      %localPoint = %parent.getLocalPoint(%worldPosition);
      %this.mount(%parent, getWord(%localPoint, 0), getWord(%localPoint, 1));
   }
}

function CIRCLE::onMouseDown(%this, %modifier, %worldPosition, %clicks) {
   if(sceneWindow2D.DRAG_OBJ == %this) {
      %this.mousePt = %worldPosition;
   }
}

function CIRCLE::onMouseDragged(%this, %modifier, %worldPosition, %clicks) {
   if(sceneWindow2D.DRAG_OBJ == %this) {
      %delta = t2dVectorSub(%worldPosition, %this.mousePt);
      %newPos = t2dVectorAdd(%this.getPosition(), %delta);
      %this.setPosition(%newPos);
      
      if(%this.getName() $= "CIRC_FULL")
         checkAABBCircleCollision(Poly_2, CIRC_FULL);
      else
      if(%this.getName() $= "CIRC_90")
         checkAABBCircle90Collision(Poly_3, CIRC_90);
      
      %this.mousePt = %worldPosition;
   }
}

function VERTICES::onMouseUp(%this, %modifier, %worldPosition, %clicks) {
   if(sceneWindow2D.DRAG_OBJ == %this) {
      %parent = %this.getMountedParent();
      %parent.drawPolygonEdges();
   }
}
function createPolygon_old() {
      %VERTICES = 5;
      for(%i = 0; %i < %VERTICES; %i++) {
         %ptA = "VERTEX_"@%i;
         %ptB = "VERTEX_"@%i+1;
         
         if(%i == %VERTICES - 1) {
            %ptB = "VERTEX_0";
         }
      
         %vectEdge = t2dVectorSub(%ptA.getPosition(), %ptB.getPosition());
         %LEN = t2dVectorDistance(%ptA.getPosition(), %ptB.getPosition());
         
         %edge = point.clone(true);
         %edge.setSize(%LEN, 1);
         %edge.setBlendColor(0, 128, 255, 255);
         
         %edgeNormalized = t2dVectorNormalise(%vectEdge);
echo("(((((((((((( %edgeNormalized = "@%edgeNormalized);
         %centerPos = t2dVectorScale(%edgeNormalized, -%LEN/2);      
         %edge.setPosition(t2dVectorAdd(%ptA.getPosition(), %centerPos));
         
         %angle = calculateAngle(%ptA.getPositionX(), %ptA.getPositionY(), %ptB.getPositionX(), %ptB.getPositionY());
         %edge.setRotation(%angle);
         
         
         %localPointVertex = Poly_0.getLocalPoint(%ptA.getPosition());
         %ptA.mount(Poly_0, getWord(%localPointVertex, 0), getWord(%localPointVertex, 1));
         
         %localPointEdge = Poly_0.getLocalPoint(%edge.getPosition());
         %edge.mount(Poly_0, getWord(%localPointEdge, 0), getWord(%localPointEdge, 1));
         /////////////////////////////////
         
         //Axis Drawing...
         %axisPoint = t2dVectorScale(%edgeNormalized, 0);
%DOT = point.clone(true);
%DOT.setPosition(t2dVectorAdd(%ptA.getPosition(), %axisPoint));
%DOT.setBlendColor(128, 0, 0, 255);
         %axis = point.clone(true);
         %axis.setSize(%LEN*5, 1);
         %axis.setBlendColor(128, 128, 128, 255);
         %axis.setRotation(90+%angle);
         %axis.setPosition(t2dVectorAdd(%ptA.getPosition(), %axisPoint));
         %localPointAxis = Poly_0.getLocalPoint(%axis.getPosition());
         %axis.mount(Poly_0, getWord(%localPointAxis, 0), getWord(%localPointAxis, 1));
         
if(%i == 0)
         calculateInterval(%axisPoint);//%axis.getPosition());
         %DOT_PROD = t2dVectorDot(t2dVectorNormalise(%axis.getPosition()), %ptA.getPosition());
         
         echo("%DOT_PROD == "@%DOT_PROD);
   }
}

function calculateInterval_old(%vectAxis) {
   %vectAxis = VERTEX_0.getPosition();
   %axisNormalised = t2dVectorNormalise(%vectAxis);
   %min = %max = t2dVectorDot(%axisNormalised, VERTEX_0.getPosition());
echo(%vectAxis@" ________________________________________ "@%axisNormalised);
      %VERTICES = 5;
      for(%i = 0; %i < %VERTICES; %i++) {
         %VERTEX = "VERTEX_"@%i;
         
         %value = t2dVectorDot(%axisNormalised, %VERTEX.getPosition());
echo(%i@" %value == "@%value);

         /*
         if(%value < %min)
            %min = %value;
         else
         if(%value > %max)
            %max = %value;
         */
         
         %LEN = t2dVectorDistance(%vectAxis, %VERTEX.getPosition());
         %DOT_PROD = t2dVectorDot(%vectAxis, %VERTEX.getPosition());
echo("%LEN == "@%LEN@" %DOT_PROD == "@%DOT_PROD);

         %tempVectorX = getWord(%vectAxis, 0)*%DOT_PROD;
         %tempVectorY = getWord(%vectAxis, 1)*%DOT_PROD;

         //%tempVectorX = %VERTEX.getPositionX()/*getWord(%axisNormalised, 0)*/*%value;
         //%tempVectorY = %VERTEX.getPositionY()/*getWord(%axisNormalised, 1)*/*%value;
         
         %m = point.clone(true);
         %m.setPosition(%tempVectorX SPC%tempVectorY);
         %m.setBlendColor(255, 0, 255, 255);
echo("POOSOOOOO________________________________________"@%m.getPosition());
         
      }
      
      
      
//echo("MIN_MAX________________________________________"@%min@" "@%max);
}

   //SAT - Collisions for Axis Aligned Bounding Boxes(AABB)
   function checkAABBCollision(%POLY_0, %POLY_1) {
         LABEL_edgePenetrationDepth.setText("----");
         
         %ALL_AXIS_COLLIDING = false;
         %DEPTH = 0;
         %NORMAL = 0;
         
         %RET_VALUE = %POLY_0.projectAxis(%POLY_1);

         %ALL_AXIS_COLLIDING = getWord(getRecord(%RET_VALUE, 0), 0);
         %SMALLEST_PENETRATION_0 = getWord(getRecord(%RET_VALUE, 0), 1);
         %PENETRATION_NORMAL_0 = getRecord(%RET_VALUE, 1);
         
         if(%ALL_AXIS_COLLIDING) {
            %RET_VALUE = %POLY_1.projectAxis(%POLY_0);
            %ALL_AXIS_COLLIDING = getWord(getRecord(%RET_VALUE, 0), 0);
            %SMALLEST_PENETRATION_1 = getWord(getRecord(%RET_VALUE, 0), 1);
            %PENETRATION_NORMAL_1 = getRecord(%RET_VALUE, 1);
         }
         
         if(%ALL_AXIS_COLLIDING) {
               if(%SMALLEST_PENETRATION_0 < %SMALLEST_PENETRATION_1) {
                  %DEPTH = -%SMALLEST_PENETRATION_0;
                  %NORMAL = %PENETRATION_NORMAL_0;
               }
               else {
                  %DEPTH = %SMALLEST_PENETRATION_1;
                  %NORMAL = %PENETRATION_NORMAL_1;
               }

               LABEL_edgePenetrationDepth.setText(%DEPTH);
               
               %polyPos = t2dVectorSub(%POLY_1.getPosition(), t2dVectorScale(%NORMAL, %DEPTH));
               %POLY_1.setPosition(%polyPos);
         }
         
         colorDOT(%ALL_AXIS_COLLIDING);
   }

   //SAT - Collisions for Axis Aligned Bounding Boxes (AABB) Vs (CIRCLE)
   function checkAABBCircleCollision(%POLYGON, %CIRCLE) {
         %ALL_AXIS_COLLIDING = false;
         %ALL_AXIS_COLLIDING = %POLYGON.projectCircle(%CIRCLE);
         
         colorDOT(%ALL_AXIS_COLLIDING);
   }
   
   //SAT - Collisions for Axis Aligned Bounding Boxes (AABB) Vs (CIRCLE)
   function checkAABBCircle90Collision(%POLYGON, %CIRCLE) {
         %ALL_AXIS_COLLIDING = false;
         %ALL_AXIS_COLLIDING = %POLYGON.projectCircle90(%CIRCLE);
         
         colorDOT(%ALL_AXIS_COLLIDING);
   }
   
   function colorDOT(%ALL_AXIS_COLLIDING) {
         if(!isObject($resultDot))
            $resultDot = point.clone(true);
         $resultDot.setSize(50, 50);
         $resultDot.setPosition(-350, -350);
         if(%ALL_AXIS_COLLIDING)
            $resultDot.setBlendColor(0, 255, 0, 255);
         else
            $resultDot.setBlendColor(255, 0, 0, 255);
   }
   