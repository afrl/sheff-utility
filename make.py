import make_helper
from make_helper import SpecializedCMakeOption
from make_helper import CMakeOptionType

specialized_opts = [
    SpecializedCMakeOption(config_var='use-fs', help_str='Turn on if compiler has <experimental/filesystem>', \
        modules_and_var_map={ 'fea_io': 'FEAIO_HaveFilesystem', 'utility': 'UTILITY_HaveFilesystem' } )
]

make_helper.make(specialized_opts)
