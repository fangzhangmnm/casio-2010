# created 20261031 by Claude Opus 5.5 — 把当年的 G1A C 源码（一个字不改）连同 fxsdk/ 里的 SDK 替身编成 WebAssembly。
# 产物：emulator/dist/g1a/<名字>.js（Emscripten MODULARIZE + SINGLE_FILE，wasm 内嵌；不进 git），build.py 再把它塞进页面。
# 需要 emsdk：环境变量 EMSDK 指过去；没设就用作者机器上的位置。用法：被 build.py 调；单独跑 = python3 -I emulator/build_g1a.py
# edited by Claude Opus 5.5 2026-10-31：源码文件夹改成相对仓根（originals/ 是当年的原件，2026/ 是 2026 版）。
import json, os, re, shutil, subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)  # 家族根（casio-2010）；edited 2026-10-31（吃书重组）：源码按子仓名去兄弟文件夹读
OUT = os.path.join(HERE, 'dist', 'g1a')
GEN = os.path.join(OUT, 'gen')  # 生成的头文件
EMSDK = os.environ.get('EMSDK') or os.path.expanduser('~/jupyter/third-party/emsdk')


# 每个 add-in：源码文件夹（相对仓根）、入口文件、导出名、要放进存储内存的只读文件（页面里 \\fls0\ 下）
ADDINS = [
    # rename：当年自己写的函数和 C 库里的同名（魔塔的 getpid = 取图块编号），编译时改个名，源码不动
    {'name': 'MAGTOWER', 'dir': '20120703 魔塔/originals/src', 'sources': ['MagTower.c'],
     'rename': ['getpid', 'pause', 'clock', 'log10']},
    # 2026 版（考古家规（见 CLAUDE.md）：原样版是默认，2026 版 = 修复版另算；user 10-31「2026版就是修复版，没必要做三个版本」）：
    # 现在 = 同一份源码，只把 pictures.h 里当年 MapEdit 转换丢掉的 103 个字节补回来；以后 user 在 WeebPaint 里优化的图块也进这一版
    {'name': 'MAGTOWER2026', 'dir': '20120703 魔塔/originals/src', 'sources': ['MagTower.c'],
     'rename': ['getpid', 'pause', 'clock', 'log10'], 'patch': {'pictures.h': '20261031 魔塔 2026/fix-mapedit-hex.json'}},
    {'name': 'DOODLE', 'dir': '20110415 Doodle Jump/originals/2013', 'sources': ['Doodle.c']},
    # 2026 版（考古家规（见 CLAUDE.md））：新写的 Doodle2026.c，以当年的 Doodle.c 为底；用 fx2026.h 的 Frame_Wait / Sensor_Read / Param_Get
    {'name': 'DOODLE2026', 'dir': '20261031 Doodle Jump 2026', 'sources': ['Doodle2026.c']},
    {'name': 'ENGINE3D', 'dir': '20111111 3D 引擎/originals', 'sources': ['3DEngine.c']},
]


def gen_headers():
    os.makedirs(GEN, exist_ok=True)
    font = json.load(open(os.path.join(HERE, 'casio-font.json'), encoding='utf-8'))
    rows = [[0] * 7 for _ in range(256)]
    for k, v in font.items():
        if k.startswith('_') or int(k, 16) > 0xFF:
            continue
        bits = bin(int(v, 16))[2:].zfill(35)
        rows[int(k, 16)] = [int(bits[r * 5:r * 5 + 5], 2) for r in range(7)]
    with open(os.path.join(GEN, 'fxfont.h'), 'w') as f:
        f.write('/* build_g1a.py 生成：casio-font.json 里单字节字符码的 5×7 字形，每行 5 位、高位在左 */\n')
        f.write('static const unsigned char FX_FONT[256][7] = {\n')
        f.write(',\n'.join('{' + ','.join(map(str, r)) + '}' for r in rows) + '};\n')
    small = json.load(open(os.path.join(HERE, 'casio-font-small.json'), encoding='utf-8'))  # PrintMini 的小字（SDK 码表第 7 页采的）
    w, g = [3] * 256, [[0] * 5 for _ in range(256)]
    for k, v in small.items():
        if not k.startswith('_'):
            w[int(k, 16)] = v['w']; g[int(k, 16)] = v['rows']
    with open(os.path.join(GEN, 'minifont.h'), 'w') as f:
        f.write('/* build_g1a.py 生成：PrintMini 小字（casio-font-small.json），每行 MINI_W 位、高位在左 */\n')
        f.write('static const unsigned char MINI_W[256] = {' + ','.join(map(str, w)) + '};\n')
        f.write('static const unsigned char MINI_ROWS[256][5] = {\n' + ',\n'.join('{' + ','.join(map(str, r)) + '}' for r in g) + '};\n')


def emcc_env():
    out = subprocess.run(['bash', '-c', f'source "{EMSDK}/emsdk_env.sh" >/dev/null 2>&1; env -0'], capture_output=True, check=True).stdout
    return dict(kv.split('=', 1) for kv in out.decode().split('\0') if '=' in kv)


# 当年 SHC 编译器认的 C89 写法（隐式 int、没声明就调用、int 和指针混用……）新版 clang 默认当错误，这里降回警告；警告不显示。
OLD_C = ['-std=gnu89', '-fno-builtin', '-w', '-Wno-error=implicit-int', '-Wno-error=implicit-function-declaration',
         '-Wno-error=int-conversion', '-Wno-error=incompatible-pointer-types', '-Wno-error=incompatible-function-pointer-types',
         '-Wno-error=return-mismatch', '-Wno-error=return-type']


def patched_copy(addin, dst):
    # 把源码复制一份再改：只替换数组里指定位置的值（[第几个元素, 新值]），别的字符原样；原件不动
    src = os.path.join(ROOT, addin['dir'])
    shutil.rmtree(dst, ignore_errors=True); os.makedirs(dst)
    for n in os.listdir(src):
        if n.endswith(('.c', '.h')): shutil.copy2(os.path.join(src, n), dst)
    for header, fixfile in addin['patch'].items():
        fix = dict(json.load(open(os.path.join(ROOT, fixfile), encoding='utf-8'))['bytes'])
        text = open(os.path.join(dst, header), encoding='latin1', newline='').read()
        a = text.index('pictures[]'); a = text.index('{', a) + 1; z = text.index('}', a)
        k = [-1]
        def sub(m):
            k[0] += 1
            return '0x%02X' % fix[k[0]] if k[0] in fix else m.group(0)
        text = text[:a] + re.sub(r'0[xX][0-9A-Fa-f]+|\d+', sub, text[a:z]) + text[z:]
        assert sum(1 for i in fix if i <= k[0]) == len(fix)
        open(os.path.join(dst, header), 'w', encoding='latin1', newline='').write(text)
    return dst


def build(addin, env):
    obj = os.path.join(OUT, addin['name'])
    os.makedirs(obj, exist_ok=True)
    src = patched_copy(addin, os.path.join(obj, 'src')) if addin.get('patch') else os.path.join(ROOT, addin['dir'])
    inc = ['-I', os.path.join(HERE, 'fxsdk'), '-I', GEN]
    cmd_sdk = ['emcc', '-O2', '-std=gnu11', '-Wall', *inc, '-c', os.path.join(HERE, 'fxsdk', 'fxsdk.c'), '-o', os.path.join(obj, 'fxsdk.o')]
    subprocess.run(cmd_sdk, env=env, check=True)
    objs = [os.path.join(obj, 'fxsdk.o')]
    for s in addin['sources']:
        o = os.path.join(obj, s + '.o')
        ren = [f'-D{n}=gwh_{n}' for n in addin.get('rename', [])]
        subprocess.run(['emcc', '-O2', *OLD_C, *ren, *inc, '-I', src, '-c', os.path.join(src, s), '-o', o], env=env, check=True)
        objs.append(o)
    js = os.path.join(OUT, addin['name'] + '.js')
    subprocess.run(['emcc', '-O2', *objs, '-o', js,
                    '-sASYNCIFY', '-sASYNCIFY_STACK_SIZE=131072', '-sMODULARIZE', f'-sEXPORT_NAME=createAddin_{addin["name"]}',
                    '-sSINGLE_FILE', '-sENVIRONMENT=web', '-sFILESYSTEM=0', '-sEXPORTED_FUNCTIONS=_fx_run',
                    '-sEXPORTED_RUNTIME_METHODS=ccall,UTF8ToString', '-sSTACK_SIZE=1048576'], env=env, check=True)
    return js


def build_all():
    if not os.path.isdir(EMSDK):
        raise SystemExit(f'没有 emsdk：{EMSDK}')
    gen_headers()
    env = emcc_env()
    return {a['name']: build(a, env) for a in ADDINS}


if __name__ == '__main__':
    for k, v in build_all().items():
        print(k, v, os.path.getsize(v), 'bytes')
