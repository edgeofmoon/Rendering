#version 330

#define SCALES 4
#define SAMPLES 32

uniform sampler2D gColor;
uniform sampler2D gNDMap;
uniform sampler2D gZoomMap;
uniform sampler2D noise;

uniform float u_colorSizeX;
uniform float u_colorSizeY;

uniform float u_lineAORadius = 2.0f;
uniform float u_lineAODepthWeight = 1.0f;
uniform float u_lineAOTotalStrength = 1.0f;

in vec2 TexCoords;
out vec4 fragColour;

vec3 lightSource = vec3( 0.0f, 0.0f, -1.0f );

float linearizeDepth(float d){
	float f = 300;
	float n = 1;
	float z = (2 * n) / (f + n - d * (f - n));
	return z;
}

float AmbientOcclusion() {

    const float falloff = 0.0001;

    float occlusion = 0.0f;

	vec3 normal = texture( gNDMap, TexCoords ).xyz;
	float currentPixelDepth = linearizeDepth(texture( gNDMap, TexCoords ).w);
	//float currentPixelDepth = texture( gNDMap, TexCoords ).w;

	vec3 ray;
	vec3 hemispherePoint;

	// grab random normal and transform to interval [-1, 1]
	vec3 randNormal = normalize( ( texture( noise, TexCoords ).xyz * 2.0f ) - vec3( 1.0f ) );

	float radiusScaler = 0.0f;
	float maxPixels = max( u_colorSizeX , u_colorSizeY );
	// beware background depth is 1
	//float radius = ( texture( gZoomMap, TexCoords ).x * u_lineAORadius / maxPixels ) / ( 1.000001f - linearizeDepth(currentPixelDepth) );
	float radius = ( texture( gZoomMap, TexCoords ).x * u_lineAORadius / maxPixels ) / ( 1.000001f - currentPixelDepth );

	//float radius = ( texture( gZoomMap, TexCoords ).x * u_lineAORadius / maxPixels );
	//float radius = ( u_lineAORadius / maxPixels );

	vec3 occluderNormal;
	float occluderDepth;
	float depthDifference;
	float normalDifference;

	for( int l=0; l<SCALES; l++ ) {

		float occluseionStep = 0.0f;

		#define radScaleMin 1.5
		radiusScaler += radScaleMin + l;
		//radiusScaler += radScaleMin;

		int numSamplesAdded = 0;
		for( int i=0; i<SAMPLES; i++ ) {

			// visibility
			vec3 randSphereNormal = ( texture( noise, 
				vec2( float( i ) / float( SAMPLES ), 
				float( l + 1 ) / float( SCALES ) ) ).rgb * 2.0f ) - vec3( 1.0f );
			vec3 hemisphereVector = reflect( randSphereNormal, randNormal );			
			ray = radiusScaler * radius * hemisphereVector;
			ray *= sign( dot( ray, normal ) );

			hemispherePoint = ray + vec3( TexCoords, currentPixelDepth );

           if( ( hemispherePoint.x < 0.0 ) || ( hemispherePoint.x > 1.0 ) ||
                ( hemispherePoint.y < 0.0 ) || ( hemispherePoint.y > 1.0 )
              )
            {
                continue;
            }

			numSamplesAdded++;

			
			// weight 	might be occluderNormal instead of hemisphereVector
			float pointDiffuse = max( dot( hemisphereVector, normal ), 0.0f );

			// illumination weight
			occluderNormal = texture( gNDMap, hemispherePoint.xy ).xyz;
			float occluderDiffuse = 0.0f; // replaceable with more realistic effect
			vec3 H = normalize( lightSource + normalize( hemisphereVector ) );
			float occluderSpecular = pow( max( dot( H, occluderNormal ), 0.0f ), 100.0 );

			normalDifference = pointDiffuse * ( occluderSpecular + occluderDiffuse );
			normalDifference = 1.5f - normalDifference;

			// depth weight
			occluderDepth = texture( gNDMap, hemispherePoint.xy ).w;
			occluderDepth = linearizeDepth(occluderDepth);
			depthDifference = currentPixelDepth - occluderDepth;

			float scaler = 1.0 - ( l / ( float( SCALES - 1 ) ) );
			float depthInfluence = scaler * scaler * u_lineAODepthWeight;
			float depthWeight = 1.0f - smoothstep( falloff, depthInfluence, depthDifference );

			occluseionStep += normalDifference * depthWeight * step( falloff, depthDifference );
		}

		occlusion += ( 1.0f / float( numSamplesAdded ) ) * occluseionStep;

	}

	float occlusionScaleFactor = 1.0f / SCALES;
	occlusionScaleFactor *= u_lineAOTotalStrength;

	return clamp( ( 1.0f - ( occlusionScaleFactor * occlusion ) ), 0, 1 );

}


void main() {
	//fragColour.xyz = texture( gColor, TexCoords ).rgb * AmbientOcclusion();
	//fragColour.w = texture( gColor, TexCoords ).a;
	//fragColour = blur();

	//fragColour = texture( gZoomMap, TexCoords );
	//fragColour = texture( gNDMap, TexCoords );
	//float currentPixelDepth = linearizeDepth(texture( gNDMap, TexCoords ).w);
	//fragColour.xyz = vec3(currentPixelDepth,currentPixelDepth,currentPixelDepth);
	float alpha = texture( gColor, TexCoords ).a;
	if(alpha == 0) discard;
	float ao = AmbientOcclusion();
	fragColour = vec4(ao,ao,ao,alpha);
}
