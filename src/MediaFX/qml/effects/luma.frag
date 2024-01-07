// Copyright (C) 2024 Andrew Wason
//
// This file is part of mediaFX.
//
// mediaFX is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// mediaFX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with mediaFX.
// If not, see <https://www.gnu.org/licenses/>.
#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float time;
    float transitionWidth;
    float premultipliedTransitionWidth;
};
layout(binding = 1) uniform sampler2D source;
layout(binding = 2) uniform sampler2D dest;
layout(binding = 3) uniform sampler2D luma;
void main() {
    vec4 sp = texture(source, qt_TexCoord0);
    vec4 dp = texture(dest, qt_TexCoord0);
    vec4 lp = texture(luma, qt_TexCoord0);
    

    vec4 m = clamp((lp * vec4(transitionWidth) - vec4(transitionWidth)) + vec4(premultipliedTransitionWidth), vec4(0.0), vec4(1.0));
    fragColor = mix(sp, dp, m) * qt_Opacity;


/*
    float m = clamp((lp.r * transitionWidth - transitionWidth) + (premultipliedTransitionWidth), 0.0, 1.0);
    fragColor = mix(sp, dp, m) * qt_Opacity;
*/

    //fragColor = vec4(lp.rgb, 1.0);
    //fragColor = vec4(mix(sp.rgb, dp.rgb, m.rgb), 1.0) * qt_Opacity;
    //fragColor = vec4(1.0, 0.0, 0.0, 1.0);

/*XXX
    float w = PREFIX(transition_width);
	float luma = INPUT3(tc).x;
	if (PREFIX(bool_inverse)) {
		luma = 1.0 - luma;
	}
	float m = clamp((luma * w - w) + PREFIX(progress_mul_w_plus_one), 0.0, 1.0);
	return mix(first, second, m);
*/
}