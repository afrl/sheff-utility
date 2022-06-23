#! /usr/bin/python3

import make_helper
from make_helper import SpecializedEnvVarOption
from make_helper import SpecializedCMakeOption
from make_helper import SpecializedOptionType

specialized_opts = [

    SpecializedCMakeOption( config_var='use-fs-hack', help_str='Turn on if compiler does not provide <experimental/filesystem>', 
                            option_type=SpecializedOptionType.CMakeBool,
                            modules_to_vars_map={ 'utility': ['SHEFF_UseFilesystemHack'] } ),
    SpecializedCMakeOption( config_var='use-mpi', help_str='Build with MPI (requires MPI environment variables for CMake to find it)', 
                            option_type=SpecializedOptionType.CMakeBool,
                            modules_to_vars_map={ 'utility': ['SHEFF_USE_MPI'] } ),

    SpecializedCMakeOption(config_var='bind-python', help_str='Build Python bindings for module',
                           option_type=SpecializedOptionType.CMakeBool,
                           modules_to_vars_map={'utility': ['SHEFF_BuildPython']}),

    SpecializedCMakeOption(config_var='bind-fortran', help_str='Build FORTRAN bindings for module',
                           option_type=SpecializedOptionType.CMakeBool,
                           modules_to_vars_map={'utility': ['SHEFF_BuildFortran']})
                           
]

make_helper.make(specialized_opts)
