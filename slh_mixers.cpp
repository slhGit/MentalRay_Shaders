#include "slh_aux.h"

//
//MIX COLORS
//

extern "C" DLLEXPORT
int slh_mix_colors_version(void) { return 1; }


struct slh_mix_colors
{
	miColor     A;
    miColor     B;
    miScalar    F;
};

extern "C" DLLEXPORT
miBoolean slh_mix_colors (miColor *result, miState *state, struct slh_mix_colors *params )
{
    miScalar F= *mi_eval_scalar(&params->F);
    if (F>1.0){F=1.0;}
    else if (F<0.0){F=0.0;}
    
    if (F == 1.0)
    {
        *result = *mi_eval_color(&params->A);
    }
    else if (F == 0.0)
    {
        *result = *mi_eval_color(&params->B);
    }
    else
    {
        miColor A = *mi_eval_color(&params->A);
        miColor B = *mi_eval_color(&params->B);
        miScalar scale = 1.0 - F;
        
        *result = (A * F) + (B * scale);
    }
    
	return miTRUE;
}


//
//MIX SCALAR
//

extern "C" DLLEXPORT
int slh_mix_scalars_version(void) { return 1; }

struct slh_mix_scalars
{
	miScalar    A;
    miScalar    B;
    miScalar    F;
};

extern "C" DLLEXPORT
miBoolean slh_mix_scalars (miScalar *result, miState *state, struct slh_mix_scalars *params )
{
    miScalar F= *mi_eval_scalar(&params->F);
    if (F>1.0){F=1.0;}
    else if (F<0.0){F=0.0;}
    
    if (F == 1.0)
    {
        *result = *mi_eval_scalar(&params->A);
    }
    else if (F == 0.0)
    {
        *result = *mi_eval_scalar(&params->B);
    }
    else
    {
        miScalar A = *mi_eval_scalar(&params->A);
        miScalar B = *mi_eval_scalar(&params->B);

        
        *result = (A * F) + (B * (1.0 - F));
    }
    
	return miTRUE;
}

//
//MIX BOOLEANS
//

extern "C" DLLEXPORT
int slh_mix_booleans_version(void) { return 1; }

struct slh_mix_booleans
{
	miBoolean    A;
    miBoolean    B;
    miScalar    F;
};

extern "C" DLLEXPORT
miBoolean slh_mix_booleans(miBoolean *result, miState *state, struct slh_mix_booleans *params )
{
    miScalar F= *mi_eval_scalar(&params->F);
    
    if (F > 0.5)
    {
        *result = *mi_eval_boolean(&params->A);
    }
    else
    {
        *result = *mi_eval_boolean(&params->B);
    }
    
	return miTRUE;
}

//
//MIX INT
//

extern "C" DLLEXPORT
int slh_mix_int_version(void) { return 1; }

struct slh_mix_int
{
    int         A;
    int         B;
    miScalar    F;
};

extern "C" DLLEXPORT
miBoolean slh_mix_int(int *result, miState *state, struct slh_mix_int *params)
{
    miScalar F = *mi_eval_scalar(&params->F);
    if (F>1.0){F=1.0;}
    else if (F<0.0){F=0.0;}
    
    if (F == 1.0)
        {*result = *mi_eval_integer(&params->A);}
    else if
        (F == 0.0) {*result = *mi_eval_integer(&params->B);}
    else
    {
        int A = *mi_eval_integer(&params->A);
        int B = *mi_eval_integer(&params->B);
        *result = (A * F) + (B * (1.0 - F));
    }
    return miTRUE;
}

//
//MIX MIA
//

//VARIABLE MIXERS
miScalar mixScalar(miScalar A, miScalar B, miScalar mask, miScalar blend)
{
    miScalar result = (A*mask) + (B*blend);
    return result;
}

miColor mixColor(miColor A, miColor B, miScalar mask, miScalar blend)
{
    miColor result;
    result.r = (A.r*mask) + (B.r*blend);
    result.g = (A.g*mask) + (B.g*blend);
    result.b = (A.b*mask) + (B.b*blend);
    result.a = (A.a*mask) + (B.a*blend);
    return result;
}

int mixInt(int A, int B, miScalar mask, miScalar blend)
{
    int result = (A*mask) + (B*blend);
    return result;
}

miBoolean mixBool(miBoolean A, miBoolean B, miScalar mask)
{
    if (mask > 0.5) { return A; }
    else { return B; }
}

void compressMask(miScalar *mask, miScalar compression)
{
    *mask = ((*mask-compression)/(1.0-2*compression));
    
    //return newmin + ((v - oldmin) / (oldmax - oldmin)) * (newmax - newmin);
}

//MIX ALL MIA ATTRIBUTES
extern "C" DLLEXPORT
int slh_mix_mia_version(void) { return 1; }

struct slh_mix_mia
{
    miScalar    diffuse_weight;
    miColor     diffuse;
    miScalar    diffuse_roughness;
    
    miScalar    reflectivity;
    miColor     refl_color;
    miScalar    refl_gloss;
    int         refl_gloss_samples;
    miBoolean   refl_interpolate;
    miBoolean   refl_hl_only;
    miBoolean   refl_is_metal;
    
    miScalar    transparency;
    miColor     refr_color;
    miScalar    refr_gloss;
    miScalar    refr_ior;
    int         refr_gloss_samples;
    miBoolean   refr_interpolate;
    miBoolean   refr_translucency;
    miColor     refr_trans_color;
    miScalar    refr_trans_weight;
    
    miScalar    anisotropy;
    miScalar    anisotropy_rotation;
    int         anisotropy_channel;
    
    miBoolean   brdf_fresnel;
    miScalar    brdf_0_degree_refl;
    miScalar    brdf_90_degree_refl;
    miScalar    brdf_curve;
    miBoolean   brdf_conserve_energy;
    
    // Reflection/Refraction optimizations & falloffs
    
    miBoolean   refl_falloff_on;
    miScalar    refl_falloff_dist;
    miBoolean   refl_falloff_color_on;
    miColor     refl_falloff_color;
    int         refl_depth;
    miScalar    refl_cutoff;
    
    miBoolean   refr_falloff_on;
    miScalar    refr_falloff_dist;
    miBoolean   refr_falloff_color_on;
    miColor     refr_falloff_color;
    int         refr_depth;
    miScalar    refr_cutoff;
    
    // Built in AO
    
    miBoolean   ao_on;
    int         ao_samples;
    miScalar    ao_distance;
    miColor     ao_dark;
    miColor     ao_ambient;
    int         ao_do_details;
    
    // Options
    
    miBoolean   thin_walled;
    miBoolean   no_visible_area_hl;
    miBoolean   skip_inside_refl;
    miBoolean   do_refractive_caustics;
    miBoolean   backface_cull;
    miBoolean   propagate_alpha;
    
    // Other effects
    
    miScalar    hl_vs_refl_balance;
    miScalar    cutout_opacity;
    miColor     additional_color;
};

struct slh_mix_mia_in
{
    miScalar    mask;
    struct      slh_mix_mia A;
    struct      slh_mix_mia B;
};

extern "C" DLLEXPORT
miBoolean slh_mix_mia(struct slh_mix_mia *result, miState *state, struct slh_mix_mia_in *params)
{
    miScalar mask = *mi_eval_scalar(&params->mask);
    if (mask>1.0){mask=1.0;}
    else if (mask<0.0){mask=0.0;}
    
    mask = (mask-0.05)/0.9;

    //mi_info("mixer_called");
    if (mask >= 1.0)
    {
        result->diffuse_weight = *mi_eval_scalar(&params->A.diffuse_weight);
        result->diffuse = *mi_eval_color(&params->A.diffuse);
        result->diffuse_roughness = *mi_eval_scalar(&params->A.diffuse_roughness);
        
        result->reflectivity = *mi_eval_scalar(&params->A.reflectivity);
        result->refl_color = *mi_eval_color(&params->A.refl_color);
        result->refl_gloss = *mi_eval_scalar(&params->A.refl_gloss);
        result->refl_gloss_samples = *mi_eval_integer(&params->A.refl_gloss_samples);
        result->refl_interpolate = *mi_eval_boolean(&params->A.refl_interpolate);
        result->refl_hl_only = *mi_eval_boolean(&params->A.refl_hl_only);
        result->refl_is_metal = *mi_eval_boolean(&params->A.refl_is_metal);
        
        result->transparency = *mi_eval_scalar(&params->A.transparency);
        result->refr_color = *mi_eval_color(&params->A.refr_color);
        result->refr_gloss = *mi_eval_scalar(&params->A.refr_gloss);
        result->refr_ior = *mi_eval_scalar(&params->A.refr_ior);
        result->refr_gloss_samples = *mi_eval_integer(&params->A.refr_gloss_samples);
        result->refr_interpolate = *mi_eval_boolean(&params->A.refr_interpolate);
        result->refr_translucency = *mi_eval_boolean(&params->A.refr_translucency);
        result->refr_trans_color = *mi_eval_color(&params->A.refr_trans_color);
        result->refr_trans_weight = *mi_eval_scalar(&params->A.refr_trans_weight);
        
        result->anisotropy = *mi_eval_scalar(&params->A.anisotropy);
        result->anisotropy_rotation = *mi_eval_scalar(&params->A.anisotropy_rotation);
        result->anisotropy_channel = *mi_eval_integer(&params->A.anisotropy_channel);
        
        result->brdf_fresnel = *mi_eval_boolean(&params->A.brdf_fresnel);
        result->brdf_0_degree_refl = *mi_eval_scalar(&params->A.brdf_0_degree_refl);
        result->brdf_90_degree_refl = *mi_eval_scalar(&params->A.brdf_90_degree_refl);
        result->brdf_curve = *mi_eval_scalar(&params->A.brdf_curve);
        result->brdf_conserve_energy = *mi_eval_boolean(&params->A.brdf_conserve_energy);
        
        // Reflection/Refraction optimizations & falloffs
        
        result->refl_falloff_on = *mi_eval_boolean(&params->A.refl_falloff_on);
        result->refl_falloff_dist = *mi_eval_scalar(&params->A.refl_falloff_dist);
        result->refl_falloff_color_on = *mi_eval_boolean(&params->A.refl_falloff_color_on);
        result->refl_falloff_color = *mi_eval_color(&params->A.refr_trans_color);
        result->refl_depth = *mi_eval_integer(&params->A.refl_depth);
        result->refl_cutoff = *mi_eval_scalar(&params->A.refl_cutoff);
        
        result->refr_falloff_on = *mi_eval_boolean(&params->A.refr_falloff_on);
        result->refr_falloff_dist = *mi_eval_scalar(&params->A.refr_falloff_dist);
        result->refr_falloff_color_on = *mi_eval_boolean(&params->A.refr_falloff_color_on);
        result->refr_falloff_color = *mi_eval_color(&params->A.refr_falloff_color);
        result->refr_depth = *mi_eval_integer(&params->A.refr_depth);
        result->refr_cutoff = *mi_eval_scalar(&params->A.refr_cutoff);
        
        // Built in AO
        
        result->ao_on = *mi_eval_boolean(&params->A.ao_on);
        result->ao_samples = *mi_eval_integer(&params->A.ao_samples);
        result->ao_distance = *mi_eval_scalar(&params->A.ao_distance);
        result->ao_dark = *mi_eval_color(&params->A.ao_dark);
        result->ao_ambient = *mi_eval_color(&params->A.ao_ambient);
        result->ao_do_details = *mi_eval_integer(&params->A.ao_do_details);
        
        // Options
        
        result->thin_walled = *mi_eval_boolean(&params->A.thin_walled);
        result->no_visible_area_hl = *mi_eval_boolean(&params->A.no_visible_area_hl);
        result->skip_inside_refl = *mi_eval_boolean(&params->A.skip_inside_refl);
        result->do_refractive_caustics = *mi_eval_boolean(&params->A.do_refractive_caustics);
        result->backface_cull = *mi_eval_boolean(&params->A.backface_cull);
        result->propagate_alpha = *mi_eval_boolean(&params->A.propagate_alpha);
        
        // Other effects
        
        result->hl_vs_refl_balance = *mi_eval_scalar(&params->A.hl_vs_refl_balance);
        result->cutout_opacity = *mi_eval_scalar(&params->A.cutout_opacity);
        result->additional_color = *mi_eval_color(&params->A.additional_color);
    }
    else if (mask <= 0.0)
    {
        result->diffuse_weight = *mi_eval_scalar(&params->B.diffuse_weight);
        result->diffuse = *mi_eval_color(&params->B.diffuse);
        result->diffuse_roughness = *mi_eval_scalar(&params->B.diffuse_roughness);
        
        result->reflectivity = *mi_eval_scalar(&params->B.reflectivity);
        result->refl_color = *mi_eval_color(&params->B.refl_color);
        result->refl_gloss = *mi_eval_scalar(&params->B.refl_gloss);
        result->refl_gloss_samples = *mi_eval_integer(&params->B.refl_gloss_samples);
        result->refl_interpolate = *mi_eval_boolean(&params->B.refl_interpolate);
        result->refl_hl_only = *mi_eval_boolean(&params->B.refl_hl_only);
        result->refl_is_metal = *mi_eval_boolean(&params->B.refl_is_metal);
        
        result->transparency = *mi_eval_scalar(&params->B.transparency);
        result->refr_color = *mi_eval_color(&params->B.refr_color);
        result->refr_gloss = *mi_eval_scalar(&params->B.refr_gloss);
        result->refr_ior = *mi_eval_scalar(&params->B.refr_ior);
        result->refr_gloss_samples = *mi_eval_integer(&params->B.refr_gloss_samples);
        result->refr_interpolate = *mi_eval_boolean(&params->B.refr_interpolate);
        result->refr_translucency = *mi_eval_boolean(&params->B.refr_translucency);
        result->refr_trans_color = *mi_eval_color(&params->B.refr_trans_color);
        result->refr_trans_weight = *mi_eval_scalar(&params->B.refr_trans_weight);
        
        result->anisotropy = *mi_eval_scalar(&params->B.anisotropy);
        result->anisotropy_rotation = *mi_eval_scalar(&params->B.anisotropy_rotation);
        result->anisotropy_channel = *mi_eval_integer(&params->B.anisotropy_channel);
        
        result->brdf_fresnel = *mi_eval_boolean(&params->B.brdf_fresnel);
        result->brdf_0_degree_refl = *mi_eval_scalar(&params->B.brdf_0_degree_refl);
        result->brdf_90_degree_refl = *mi_eval_scalar(&params->B.brdf_90_degree_refl);
        result->brdf_curve = *mi_eval_scalar(&params->B.brdf_curve);
        result->brdf_conserve_energy = *mi_eval_boolean(&params->B.brdf_conserve_energy);
        
        // Reflection/Refraction optimizations & falloffs
        
        result->refl_falloff_on = *mi_eval_boolean(&params->B.refl_falloff_on);
        result->refl_falloff_dist = *mi_eval_scalar(&params->B.refl_falloff_dist);
        result->refl_falloff_color_on = *mi_eval_boolean(&params->B.refl_falloff_color_on);
        result->refl_falloff_color = *mi_eval_color(&params->B.refr_trans_color);
        result->refl_depth = *mi_eval_integer(&params->B.refl_depth);
        result->refl_cutoff = *mi_eval_scalar(&params->B.refl_cutoff);
        
        result->refr_falloff_on = *mi_eval_boolean(&params->B.refr_falloff_on);
        result->refr_falloff_dist = *mi_eval_scalar(&params->B.refr_falloff_dist);
        result->refr_falloff_color_on = *mi_eval_boolean(&params->B.refr_falloff_color_on);
        result->refr_falloff_color = *mi_eval_color(&params->B.refr_falloff_color);
        result->refr_depth = *mi_eval_integer(&params->B.refr_depth);
        result->refr_cutoff = *mi_eval_scalar(&params->B.refr_cutoff);
        
        // Built in AO
        
        result->ao_on = *mi_eval_boolean(&params->B.ao_on);
        result->ao_samples = *mi_eval_integer(&params->B.ao_samples);
        result->ao_distance = *mi_eval_scalar(&params->B.ao_distance);
        result->ao_dark = *mi_eval_color(&params->B.ao_dark);
        result->ao_ambient = *mi_eval_color(&params->B.ao_ambient);
        result->ao_do_details = *mi_eval_integer(&params->B.ao_do_details);
        
        // Options
        
        result->thin_walled = *mi_eval_boolean(&params->B.thin_walled);
        result->no_visible_area_hl = *mi_eval_boolean(&params->B.no_visible_area_hl);
        result->skip_inside_refl = *mi_eval_boolean(&params->B.skip_inside_refl);
        result->do_refractive_caustics = *mi_eval_boolean(&params->B.do_refractive_caustics);
        result->backface_cull = *mi_eval_boolean(&params->B.backface_cull);
        result->propagate_alpha = *mi_eval_boolean(&params->B.propagate_alpha);
        
        // Other effects
        
        result->hl_vs_refl_balance = *mi_eval_scalar(&params->B.hl_vs_refl_balance);
        result->cutout_opacity = *mi_eval_scalar(&params->B.cutout_opacity);
        result->additional_color = *mi_eval_color(&params->B.additional_color);

    }
    else
    {
        miScalar blend = 1.0 - mask;
        
        result->diffuse_weight = mixScalar(*mi_eval_scalar(&params->A.diffuse_weight),*mi_eval_scalar(&params->B.diffuse_weight), mask, blend);
        result->diffuse = mixColor(*mi_eval_color(&params->A.diffuse), *mi_eval_color(&params->B.diffuse), mask, blend);
        result->diffuse_roughness = mixScalar(*mi_eval_scalar(&params->A.diffuse_roughness),*mi_eval_scalar(&params->B.diffuse_roughness), mask, blend);
        
        result->reflectivity = mixScalar(*mi_eval_scalar(&params->A.reflectivity),*mi_eval_scalar(&params->B.reflectivity), mask, blend);
        result->refl_color = mixColor(*mi_eval_color(&params->A.refl_color),*mi_eval_color(&params->B.refl_color), mask, blend);
        result->refl_gloss = mixScalar(*mi_eval_scalar(&params->A.refl_gloss),*mi_eval_scalar(&params->B.refl_gloss), mask, blend);
        result->refl_gloss_samples = mixInt(*mi_eval_integer(&params->A.refl_gloss_samples),*mi_eval_integer(&params->B.refl_gloss_samples), mask, blend);
        result->refl_interpolate = mixBool(*mi_eval_boolean(&params->A.refl_interpolate),*mi_eval_boolean(&params->B.refl_interpolate), mask);
        result->refl_hl_only = mixBool(*mi_eval_boolean(&params->A.refl_hl_only),*mi_eval_boolean(&params->B.refl_hl_only), mask);
        result->refl_is_metal = mixBool(*mi_eval_boolean(&params->A.refl_is_metal),*mi_eval_boolean(&params->B.refl_is_metal), mask);
        
        result->transparency = mixScalar(*mi_eval_scalar(&params->A.transparency),*mi_eval_scalar(&params->B.transparency), mask, blend);
        result->refr_color = mixColor(*mi_eval_color(&params->A.refr_color),*mi_eval_color(&params->B.refr_color), mask, blend);
        result->refr_gloss = mixScalar(*mi_eval_scalar(&params->A.refr_gloss),*mi_eval_scalar(&params->B.refr_gloss), mask, blend);
        result->refr_ior = mixScalar(*mi_eval_scalar(&params->A.refr_ior),*mi_eval_scalar(&params->B.refr_ior), mask, blend);
        result->refr_gloss_samples = mixInt(*mi_eval_integer(&params->A.refr_gloss_samples),*mi_eval_integer(&params->B.refr_gloss_samples), mask, blend);
        result->refr_interpolate = mixBool(*mi_eval_boolean(&params->A.refr_interpolate),*mi_eval_boolean(&params->B.refr_interpolate), mask);
        result->refr_translucency = mixBool(*mi_eval_boolean(&params->A.refr_translucency),*mi_eval_boolean(&params->B.refr_translucency), mask);
        result->refr_trans_color = mixColor(*mi_eval_color(&params->A.refr_trans_color),*mi_eval_color(&params->B.refr_trans_color), mask, blend);
        result->refr_trans_weight = mixScalar(*mi_eval_scalar(&params->A.refr_trans_weight),*mi_eval_scalar(&params->B.refr_trans_weight), mask, blend);
        
        result->anisotropy = mixScalar(*mi_eval_scalar(&params->A.anisotropy),*mi_eval_scalar(&params->B.anisotropy), mask, blend);
        result->anisotropy_rotation = mixScalar(*mi_eval_scalar(&params->A.anisotropy_rotation),*mi_eval_scalar(&params->B.anisotropy_rotation), mask, blend);
        result->anisotropy_channel = mixInt(*mi_eval_integer(&params->A.anisotropy_channel),*mi_eval_integer(&params->B.anisotropy_channel), mask, blend);
        
        result->brdf_fresnel = mixBool(*mi_eval_boolean(&params->A.brdf_fresnel),*mi_eval_boolean(&params->B.brdf_fresnel), mask);
        result->brdf_0_degree_refl = mixScalar(*mi_eval_scalar(&params->A.brdf_0_degree_refl),*mi_eval_scalar(&params->B.brdf_0_degree_refl), mask, blend);
        result->brdf_90_degree_refl = mixScalar(*mi_eval_scalar(&params->A.brdf_90_degree_refl),*mi_eval_scalar(&params->B.brdf_90_degree_refl), mask, blend);
        result->brdf_curve = mixScalar(*mi_eval_scalar(&params->A.brdf_curve),*mi_eval_scalar(&params->B.brdf_curve), mask, blend);
        result->brdf_conserve_energy = mixBool(*mi_eval_boolean(&params->A.brdf_conserve_energy),*mi_eval_boolean(&params->B.brdf_conserve_energy), mask);
        
        // Reflection/Refraction optimizations & falloffs
        
        result->refl_falloff_on = mixBool(*mi_eval_boolean(&params->A.refl_falloff_on),*mi_eval_boolean(&params->B.refl_falloff_on), mask);
        result->refl_falloff_dist = mixScalar(*mi_eval_scalar(&params->A.refl_falloff_dist),*mi_eval_scalar(&params->B.refl_falloff_dist), mask, blend);
        result->refl_falloff_color_on = mixBool(*mi_eval_boolean(&params->A.refl_falloff_color_on),*mi_eval_boolean(&params->B.refl_falloff_color_on), mask);
        result->refl_falloff_color = mixColor(*mi_eval_color(&params->A.refr_trans_color),*mi_eval_color(&params->B.refr_trans_color), mask, blend);
        result->refl_depth = mixInt(*mi_eval_integer(&params->A.refl_depth),*mi_eval_integer(&params->B.refl_depth), mask, blend);
        result->refl_cutoff = mixScalar(*mi_eval_scalar(&params->A.refl_cutoff),*mi_eval_scalar(&params->B.refl_cutoff), mask, blend);
        
        result->refr_falloff_on = mixBool(*mi_eval_boolean(&params->A.refr_falloff_on),*mi_eval_boolean(&params->B.refr_falloff_on), mask);
        result->refr_falloff_dist = mixScalar(*mi_eval_scalar(&params->A.refr_falloff_dist),*mi_eval_scalar(&params->B.refr_falloff_dist), mask, blend);
        result->refr_falloff_color_on = mixBool(*mi_eval_boolean(&params->A.refr_falloff_color_on),*mi_eval_boolean(&params->B.refr_falloff_color_on), mask);
        result->refr_falloff_color = mixColor(*mi_eval_color(&params->A.refr_falloff_color),*mi_eval_color(&params->B.refr_falloff_color), mask, blend);
        result->refr_depth = mixInt(*mi_eval_integer(&params->A.refr_depth),*mi_eval_integer(&params->B.refr_depth), mask, blend);
        result->refr_cutoff = mixScalar(*mi_eval_scalar(&params->A.refr_cutoff),*mi_eval_scalar(&params->B.refr_cutoff), mask, blend);
        
        // Built in AO
        
        result->ao_on = mixBool(*mi_eval_boolean(&params->A.ao_on),*mi_eval_boolean(&params->B.ao_on), mask);
        result->ao_samples = mixInt(*mi_eval_integer(&params->A.ao_samples),*mi_eval_integer(&params->B.ao_samples), mask, blend);
        result->ao_distance = mixScalar(*mi_eval_scalar(&params->A.ao_distance),*mi_eval_scalar(&params->B.ao_distance), mask, blend);
        result->ao_dark = mixColor(*mi_eval_color(&params->A.ao_dark),*mi_eval_color(&params->B.ao_dark), mask, blend);
        result->ao_ambient = mixColor(*mi_eval_color(&params->A.ao_ambient),*mi_eval_color(&params->B.ao_ambient), mask, blend);
        result->ao_do_details = mixInt(*mi_eval_integer(&params->A.ao_do_details),*mi_eval_integer(&params->B.ao_do_details), mask, blend);
        
        // Options
        
        result->thin_walled = mixBool(*mi_eval_boolean(&params->A.thin_walled),*mi_eval_boolean(&params->B.thin_walled), mask);
        result->no_visible_area_hl = mixBool(*mi_eval_boolean(&params->A.no_visible_area_hl),*mi_eval_boolean(&params->B.no_visible_area_hl), mask);
        result->skip_inside_refl = mixBool(*mi_eval_boolean(&params->A.skip_inside_refl),*mi_eval_boolean(&params->B.skip_inside_refl), mask);
        result->do_refractive_caustics = mixBool(*mi_eval_boolean(&params->A.do_refractive_caustics),*mi_eval_boolean(&params->B.do_refractive_caustics), mask);
        result->backface_cull = mixBool(*mi_eval_boolean(&params->A.backface_cull),*mi_eval_boolean(&params->B.backface_cull), mask);
        result->propagate_alpha = mixBool(*mi_eval_boolean(&params->A.propagate_alpha),*mi_eval_boolean(&params->B.propagate_alpha), mask);
        
        // Other effects
        
        result->hl_vs_refl_balance = mixScalar(*mi_eval_scalar(&params->A.hl_vs_refl_balance),*mi_eval_scalar(&params->B.hl_vs_refl_balance), mask, blend);
        result->cutout_opacity = mixScalar(*mi_eval_scalar(&params->A.cutout_opacity),*mi_eval_scalar(&params->B.cutout_opacity), mask, blend);
        result->additional_color = mixColor(*mi_eval_color(&params->A.additional_color),*mi_eval_color(&params->B.additional_color), mask, blend);
    }
    return miTRUE;
}
