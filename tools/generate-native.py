#!/usr/bin/env python3
"""Génère l’interface C du module Go ; aucun générateur à lancer chez l’utilisateur."""
from pathlib import Path
import re
import json

ROOT = Path(__file__).resolve().parents[1]
BINDINGS = {
    'adaboost', 'approx_kfn', 'bayesian_linear_regression', 'cf', 'dbscan',
    'decision_tree', 'det', 'emst', 'fastmks', 'gmm_generate', 'gmm_probability',
    'gmm_train', 'hmm_generate', 'hmm_loglik', 'hmm_train', 'hmm_viterbi',
    'hoeffding_tree', 'image_converter', 'kde', 'kernel_pca', 'kfn', 'kmeans',
    'knn', 'krann', 'lars', 'linear_regression', 'linear_svm', 'lmnn',
    'local_coordinate_coding', 'logistic_regression', 'lsh', 'mean_shift', 'nbc',
    'nca', 'nmf', 'pca', 'perceptron', 'preprocess_binarize',
    'preprocess_describe', 'preprocess_one_hot_encoding', 'preprocess_scale',
    'preprocess_split', 'radical', 'random_forest', 'softmax_regression',
    'sparse_coding',
}


def protect(body, failure=''):
    return ('\n  try\n  {\n' + ''.join('    ' + line + '\n' for line in body.strip().splitlines()) +
            '  }\n  catch (const std::exception& error) { MilviSetError(error.what()); }\n'
            '  catch (...) { MilviSetError("exception C++ inconnue"); }\n' +
            ('  return ' + failure + ';\n' if failure else ''))


def main():
    models = {}
    found = set()
    for path in sorted((ROOT / 'src/mlpack/methods').rglob('*_main.cpp')):
        content = path.read_text()
        match = re.search(r'#define\s+BINDING_NAME\s+(\w+)', content)
        if not match or match[1] not in BINDINGS:
            continue
        name = match[1]
        found.add(name)
        camel = ''.join(word[0].upper() + word[1:] for word in name.split('_'))
        relative = path.relative_to(ROOT / 'src')
        output = ('// Généré par tools/generate-native.py ; conserver ce fichier dans Git.\n'
                  '#define BINDING_TYPE BINDING_TYPE_GO\n'
                  f'#include <{relative}>\n'
                  '#include <mlpack/bindings/go/capi/io_util.hpp>\n'
                  '#include <mlpack/bindings/go/capi/ownership.hpp>\n'
                  '#include <mlpack/bindings/go/capi/errors.hpp>\n\n')
        body = ('mlpack::util::Params& p = *static_cast<mlpack::util::Params*>(params);\n'
                'mlpack::util::Timers& t = *static_cast<mlpack::util::Timers*>(timers);\n'
                'BINDING_FUNCTION(p, t);')
        output += f'extern "C" void mlpack{camel}(void* params, void* timers)\n{{' + protect(body) + '}\n'
        types = re.findall(r'PARAM_MODEL_(?:IN|IN_REQ|OUT)\(([A-Za-z_<>]+),', content)
        for model in dict.fromkeys(types):
            safe = re.sub('[<>]', '', model)
            if safe in models:
                assert models[safe] == model
                continue
            models[safe] = model
            output += f'\nextern "C" void mlpackSet{safe}Ptr(void* params, const char* id, void* value)\n{{'
            output += protect('mlpack::bindings::go::BorrowModel(params, value);\n'
                              f'static_cast<mlpack::util::Params*>(params)->Get<{model}*>(id) = static_cast<{model}*>(value);') + '}\n'
            output += f'\nextern "C" void* mlpackGet{safe}Ptr(void* params, const char* id)\n{{'
            output += protect(f'auto* value = static_cast<mlpack::util::Params*>(params)->Get<{model}*>(id);\n'
                              'mlpack::bindings::go::ExportModel(params, value);\nreturn value;', 'nullptr') + '}\n'
            output += f'\nextern "C" void mlpackDelete{safe}Ptr(void* value)\n{{'
            output += protect(f'delete static_cast<{model}*>(value);') + '}\n'
        (ROOT / f'native_{name}.cpp').write_text(output)
    assert found == BINDINGS, sorted(BINDINGS - found)
    for util in ['arma_util', 'io_util']:
        (ROOT / f'native_{util}.cpp').write_text(
            '// Utilitaire natif commun, compilé par cgo.\n'
            f'#include <mlpack/bindings/go/capi/{util}.cpp>\n')
    (ROOT / 'native_models.json').write_text(json.dumps(models, indent=2) + '\n')
    print(f'{len(found)} méthodes ; {len(models)} types de modèles, chacun défini une seule fois.')


if __name__ == '__main__':
    main()
