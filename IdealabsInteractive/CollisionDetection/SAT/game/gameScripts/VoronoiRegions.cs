

   function getVoronoiRegionOrVertex(%POLYGON, %POINT) {
      %VERTICES = %POLYGON.noOfVertices;
      %IS_POINT_INSIDE_POLYGON = true;
      
      for(%i = 0; %i < %VERTICES; %i++) {
         %ptA = %POLYGON.getName()@"_VERTEX_"@%i;
         %ptB = %POLYGON.getName()@"_VERTEX_"@%i+1;
                  
         if(%i == %VERTICES - 1) {
            %ptB = %POLYGON.getName()@"_VERTEX_0";
            %VERTEX_B = %ptB.getPosition();
         }
      
         %VERTEX_A = %ptA.getPosition();
         %VERTEX_B = %ptB.getPosition();
         
         //Since the vertices are arranged in clockwise direction,
         //the portion to the left of the line/EDGE is NEGATIVE and 
         //that to the right is POSITIVE HALF. This gets reverse if
         //the vertice are arranged in anti-clockwise direction.
         //Now, we just need to find whether the point lies in
         // which half & then check for VoronoiRegion
         //file:///C:/Documents and Settings/aalekh.maldikar/Desktop/CollisionDetection/polygon/Polygon/docs (deprectated)/html/segment - polygon intersection.htm

         %DELTA_EDGE = t2dVectorSub(%VERTEX_A, %VERTEX_B);
         %LHNormal_EDGE = -getWord(%DELTA_EDGE, 1) SPC getWord(%DELTA_EDGE, 0);
         %DELTA_POINT_VERTEX = t2dVectorSub(%VERTEX_A, %POINT);
         %SIGN = t2dVectorDot(%DELTA_POINT_VERTEX, %LHNormal_EDGE);
         
         //(%SIGN < 0) shows that the point is facing the EDGE else point behind the EDGE - For clockwise arrangement of VERTICES.
         if(%SIGN < 0) {
            //POINT in test lies facing this EDGE...
            
            %NORMALISED_DELTA_EDGE = t2dVectorNormalise(%DELTA_EDGE);
            %MAX_PROD = t2dVectorDot(%NORMALISED_DELTA_EDGE, %DELTA_EDGE);
         
            %POINT_PROD = t2dVectorDot(%NORMALISED_DELTA_EDGE, %DELTA_POINT_VERTEX);
            
            %IS_POINT_INSIDE_POLYGON = false;
            
            if(   %POINT_PROD >= 0
                  &&
                  %POINT_PROD <= %MAX_PROD
            ) {
               //POINT in test lies in the region that bounds this EDGE...
               return (%i SPC (%i+1));
            }
         }
      }
      
      if(!%IS_POINT_INSIDE_POLYGON) {
            //POINT in test lies in the region closer to some VERTEX...
            
            //CHECK FOR VERTICES:
            %CLOSEST_VERTEX = 0;
            %RADII_VALUE = 0;
            for(%i = 0; %i < %VERTICES; %i++) {
               %ptA = %POLYGON.getName()@"_VERTEX_"@%i;
               %VERTEX_A = %ptA.getPosition();
               
               %DELTA_POINT_VERTEX = t2dVectorDistance(%VERTEX_A, %POINT);
               if(   %i == 0
                     ||
                     %DELTA_POINT_VERTEX < %RADII_VALUE      
               ){
                  %RADII_VALUE = %DELTA_POINT_VERTEX;
                  %CLOSEST_VERTEX = %i;
               }
            }
            
            return %CLOSEST_VERTEX;
      }
      
      //POINT in test lies inside the POLYGON...
      return "POINT INSIDE THE POLYGON";
   }