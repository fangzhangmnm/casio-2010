# created 20261030 by Claude Opus 5.5 — 把 originals/ 里的 G1M 原件、编好的 G1A（build_g1a.py）和字形填进 shell.html，产出仓根的 index.html（GitHub Pages 就发它）。
# 用法：python3 -I build.py（在本仓根跑）
# edited by Claude Opus 5.5 2026-10-31（吃书重组）：casio-2010 拆成家族仓，原件在各作品子仓，2026 版在各自的 2026 子仓；这里按子仓名去兄弟文件夹读，index.html 产在本仓根。
# edited by Claude Opus 5.5 2026-10-31：从私仓搬进 casio-2010，原件直接读 originals/、2026 版读 2026/，不再另存副本。
# 每个程序的说明照录当年论坛帖子里 GWHBOB 的原话（cnCalc 计算器论坛），日期 = 文件里的时间戳 / 发帖时间（北京时间）。
import base64, json, os, sys
here = os.path.dirname(os.path.abspath(__file__))
root = os.path.dirname(here)  # 家族根（casio-2010），各作品子仓是这里的兄弟文件夹
BASIC_HOME = {'SNACKI.G1M': '20100829 贪吃蛇', '21.G1M': '20100829 21点', 'CRPG.G1M': '20100918 三国杀简版',
              'TDGAME.G1M': '20101022 塔防', 'TDGAME11.G1M': '20101022 塔防', 'MINE.G1M': '20120629 扫雷', 'MINEA.G1M': '20120629 扫雷'}
PROGRAMS = [
    {'file': 'SNACKI.G1M', 'size': 952, 'title': 'SnackI 贪吃蛇（不卡版）', 'date': '2010-08-29',
     'about': '2010-08-12 在论坛挖坑：「我知道怎么编出o(n)=1 (时间复杂度)的贪吃蛇，要用到指针和循环数组，还有较大内存，就是变长之后不会卡」；08-16 贴源码，08-29 发 G1M（文件时间 21:01）。先输入速度 1–10，方向键或 8/2/4/6 转向。cnCalc 4330 / 4388 / 4573。',
     'notesTitle': '「已知缺陷」（4573 帖原话）',
     'notes': ['α. speed(1~8)设为1，2，3，4会很慢（原因：不想研究离散反比例函数）【已解决】',
               'β. speed设为11，试试看（键盘有点卡，得一直按着）',
               'γ. 食物出现在蛇身体中（故意的，而且不这样O(n)就不会为1，但吃到蛇身体中食物的会gameover，有些游戏的解决方法是食物出现在尾巴）',
               'δ. 往相反的方向行走会gameover（故意的，请小心）【已解决】',
               'ε. 最下面一排不能走，会over（fx-gX系列文本显示只有7排，最下面一排是留给标签的，请耐心等待graph版本SnackI2）',
               'ζ. 没有缺陷']},
    {'file': 'MINE.G1M', 'size': 1516, 'title': '扫雷（9860 版）', 'date': '2012-06-29',
     'about': '「基本的G1M BASIC扫雷。开始时输入雷数，建议20~40。方向键移动光标（“?”），[EXE]挖，[(-)]插旗子（[(-)]就是[EXE]左边那个键）。挖到空白时会把周围的格子全翻开的。」邻格偏移存成复数 {-1-i, …, 1+i}，用 ReP / ImP 取坐标；空白连片展开用 List 2 当队列。cnCalc 7597，2012-06-29 18:48。',
     'notesTitle': '两个版本（7597 帖原话）',
     'notes': ['「注意有两个版本，9860的版本没刷过机的9750不能用，但较美观一点」',
               '9750GII 问「是math中的筐筐的问题么?」——「对，就那个框。看来以前一些出问题的g1m大概也是这个问题了。」（9860 版用的是一个空心长方框字符）']},
    {'file': 'MINEA.G1M', 'size': 1548, 'title': '扫雷（9750 版）', 'date': '2012-06-29',
     'about': '和 9860 版同一个程序，没翻开的格子换成没刷机的 9750 也有的 □ 字符。cnCalc 7597。', 'notes': []},
]
# 每个程序的按键：controls = 每个阶段（input 输入数字 / run 进行中 / done 结束）屏幕下那行说明 + 每个键帽下写的用途（Getkey 码）；
# bind = 这个程序专门的电脑键（KeyboardEvent.code → Getkey 码）和触屏玩法（swipe = 滑屏当方向键；cell = 点格子 = 光标走过去再按 tap / 长按 = long）。
DIGITS = [74, 64, 54, 73, 63, 53, 72, 62, 52, 71]
WASD = {'KeyW': 28, 'KeyS': 37, 'KeyA': 38, 'KeyD': 27}
SNAKE = {
    'controls': {
        'input': {'title': '先定速度', 'text': '输入速度 1–10（越大越快，大于 10 = 最快），按 EXE 开始。', 'pc': '电脑：数字键 + Enter。',
                  'keys': [[DIGITS, ''], [[31], '开始'], [[44], '删']]},
        'run': {'title': '转向', 'text': '方向键（或数字 8 / 2 / 4 / 6）转向。别直接掉头，会撞到自己；最下面一行不能走（原版就这样）。触屏：在屏幕上往哪滑就往哪转。',
                'pc': '电脑：方向键或 WASD。',
                'keys': [[[28, 64], '上'], [[37, 62], '下'], [[38, 73], '左'], [[27, 53], '右']]},
        'done': {'title': 'GAME OVER', 'text': 'SCORE = 吃到的数量（原版从 1 算起）。EXE 再来一局，EXIT 回列表。点屏幕也是再来。', 'pc': '电脑：Enter / Esc。',
                 'keys': [[[31], '再来']]},
    },
    'bind': {'keyboard': WASD, 'touch': {'mode': 'swipe'}},
}
MINE_CTL = {
    'controls': {
        'input': {'title': '先定雷数', 'text': '输入雷数（作者原话「建议20~40」），按 EXE。然后会生成雷区，右上角走百分比，要等几秒。', 'pc': '电脑：数字键 + Enter。',
                  'keys': [[DIGITS, ''], [[31], '开始'], [[44], '删']]},
        'run': {'title': '挖雷', 'text': '触屏：直接点格子 = 走过去挖开；长按格子 = 插旗 / 拔旗。也可以用方向键移动光标「?」，EXE 挖，(−) 插旗。挖到 0 会自动翻开一片（要等一会儿）。',
                'pc': '电脑：左键点格子挖，右键点格子插旗；或方向键 / WASD 移动，空格 / Enter 挖，F 插旗。',
                'keys': [[[28], '移动'], [[37], '移动'], [[38], '移动'], [[27], '移动'], [[31], '挖'], [[41], '插旗']]},
        'done': {'title': '结束', 'text': 'Game Over 或 You Win。EXE 再来，EXIT 回列表。点屏幕也是再来。', 'pc': '电脑：Enter / Esc。', 'keys': [[[31], '再来']]},
    },
    'bind': {'keyboard': {**WASD, 'Space': 31, 'KeyF': 41}, 'touch': {'mode': 'cell', 'x': 'U', 'y': 'V', 'tap': [31], 'long': [41]}},
}
PROGRAMS[0].update(SNAKE); PROGRAMS[1].update(MINE_CTL); PROGRAMS[2].update(MINE_CTL)

# 规则：只写游戏本身的规则（user 10-31「别做多余的事情」），从代码里读出来的。一行一条，"## " 开头 = 小标题。
PROGRAMS[0].update({'entry': 'SNACKI', 'rules': [
    '开局先输入速度 1–10，越大越快；大于 10 = 最快。',
    '蛇一直往前走，方向键改方向；按反方向没用（不会掉头）。',
    '吃到 ¥ 变长一节。食物可能刷在蛇身上，吃到那个也算撞到自己。',
    '撞墙或撞到自己就结束。最下面一行也算墙。',
    '结束时 SCORE = 蛇的长度减 1。']})
MINE_RULES = [
    '开局输入雷数（作者建议 20~40），雷区 21 × 7。',
    '开局光标在正中那格，那格一定不是雷。',
    '挖开的格子：数字 = 周围 8 格里有几颗雷；挖到 0 会把相连的一片自动翻开。',
    '挖到雷 = Game Over。',
    '在认定是雷的格子插旗，再插一次拔掉。',
    '所有雷都插上旗、而且没有插错的旗，才算赢（You Win）；只把安全的格子全挖开不算赢。']
PROGRAMS[1].update({'entry': 'MINE', 'rules': MINE_RULES})
PROGRAMS[2].update({'entry': 'MINEA', 'rules': MINE_RULES})
FKEYS_DIGITS = {'Digit1': 79, 'Digit2': 69, 'Digit3': 59, 'Digit4': 49, 'Digit5': 39, 'Digit6': 29,
                'Numpad1': 79, 'Numpad2': 69, 'Numpad3': 59, 'Numpad4': 49, 'Numpad5': 39, 'Numpad6': 29}
PROGRAMS.append({
    'file': '21.G1M', 'size': 5044, 'entry': '21.', 'title': '21 点（CARD 21.）', 'date': '2010-08-29',
    'about': '4574 帖正文只有一句「大家看一下我的字符串应用」。最多 6 个人轮流用同一台计算器的 21 点变体，可以加电脑玩家 CASIO。文件里 4 个程序：主程序 21.、摸牌 21.MORE、结算 21.S、等按键 GETKEY。cnCalc 4574，2010-08-29 21:16。',
    'notes': [],
    'rules': [
        '## 开局',
        '6 个座位挨个问：1 = 人类（再输入名字，起始 100 元），2 = 电脑 CASIO（起始 150 元），3 = 空位。',
        '## 牌',
        '104 张，每个点数 8 张，不分花色。A = 1，2–10 照面值，J = 11、Q = 12、K = 13。',
        '牌摸光了自动重洗。',
        '## 轮到你',
        '先依次按 F1 到 F6，把盖住屏幕的幕布一段段揭开。',
        'F1 摸一张；F2 看别人上一回合的消息；F3 看各人的钱；F4 结束回合。',
        '结束回合时手里的牌留着，下一轮接着攒。',
        '## 结算（手里的点数加起来到 21 或以上时立刻发生）',
        '正好 21：每个还活着的对手付你 5 元。',
        '超过 21：你付每个还活着的对手 1 元。',
        '结算完所有人的手牌一起清空。',
        '钱到 0 或以下就死了，之后跳过。',
        '## 电脑',
        '电脑每回合摸一张，然后结束回合。',
        '## 结束',
        '游戏没有结局，玩够了按 EXIT。',
        '## 攻略：什么时候该摸（规则里没写，按代码算的）',
        '赢是 +5，输只是 −1，所以这个游戏鼓励你摸。',
        '手里现在 t 点，再摸一张：正好 21 的概率约 1/13，爆的概率是比 21 − t 大的那些点数。按牌堆均匀算，摸这一张对每个对手的期望是 (13 − t) ÷ 13 元。',
        't ≤ 7：爆不了，0；t = 8：+0.38；9：+0.31；10：+0.23；11：+0.15；12：+0.08；13：0；14：−0.08；16：−0.23；18：−0.38；20：−0.54。',
        '一句话：手里 12 点以下就摸，14 点以上就停。这只算了眼前这一张，没算后面几轮，也没算别人凑到 21 会把你攒的牌清掉。电脑不管多少都摸，所以它手大的时候平均在送钱。'],
    'controls': {
        'input': {'title': '开局设置', 'text': '每个座位输入 1（人类）/ 2（电脑）/ 3（空位），按 EXE。', 'pc': '电脑：数字键 + Enter。',
                  'keys': [[DIGITS, ''], [[31], '确定'], [[44], '删']]},
        'inputStr': {'title': '输入名字', 'text': '在屏幕下面的输入框里打名字（字母 / 数字，会变成大写），按「确定」。', 'pc': '电脑：直接打字，Enter 确定。',
                     'keys': [[[31], '确定']]},
        'run': {'title': '轮流出牌', 'text': '换人时依次按 F1 → F6 揭开幕布。然后 F1 摸牌、F2 看消息、F3 看各人的钱、F4 结束回合。屏幕上出现「You get a […]」或消息时，按任意键继续。',
                'pc': '电脑：数字键 1–6 = F1–F6（键盘上的 F1–F6 也行），Enter = 任意键。',
                'keys': [[[79], '摸牌'], [[69], '消息'], [[59], '排行'], [[49], '结束'], [[39], '揭幕'], [[29], '揭幕'], [[31], '继续']]},
    },
    'bind': {'keyboard': FKEYS_DIGITS, 'touch': {'mode': 'swipe'}},
})

# 三国杀简版、塔防：要图形屏（Text / DrawStat / Pict / Menu），10-31 补上；画法对过塔防 5009 帖的 5 张截图（见 ai-docs）
PROGRAMS.append({
    'file': 'CRPG.G1M', 'size': 3492, 'entry': 'CRPG', 'title': '三国杀简版（CRPG）', 'date': '2010-09-18',
    'about': '「【.G1M】【CRPG】三国杀简版，只支持杀，闪，桃」——正文就一句「空虚的时候玩玩吧！」。你对电脑 ZZM，一人 5 点血。文件里还有两个画图的试验程序（AAAAAAAA、DDDDDDDD），主程序不调它们。cnCalc 4743，2010-09-18 11:23。',
    'notes': [],
    'rules': [
        '## 开局',
        '你和 ZZM 各 5 点血，各随机摸 4 张牌。牌只有三种：杀（Kill）、闪（Miss）、桃（Heal）。',
        '## 你的回合',
        '先摸 2 张。屏幕上是双方的血，接着问「END?」：No = 出牌，Yes = 结束回合。',
        '出牌时列出三种牌各有几张，选一张用；选 Return 回去。',
        '杀：一回合只能用一次。ZZM 手里有闪就自动闪掉，没有就掉 1 点血。',
        '闪：自己出没有用（也不消耗），只在挨杀时自动替你挡。',
        '桃：回 1 点血；血满 5 点时不能用。',
        '结束回合时手牌比血多，就得弃牌，弃到不超过血量为止。',
        '## ZZM 的回合',
        'ZZM 摸 2 张；手里有杀的话一半机会杀你，你有闪就自动闪掉，没有就掉 1 点血。手牌比血多就随机弃牌。',
        '## 输赢',
        '谁掉到 0 点血时手里有桃，自动用一张回到 1 点；没有桃就输了——你输是「You LOST???」，ZZM 输是「You W.I.N!」。'],
    'controls': {
        'run': {'title': '三国杀', 'text': '屏幕上出一句话就停下来等你按 EXE；选单用 ↑↓ + EXE，或直接按数字键、点屏幕上那一项。', 'pc': '电脑：Enter、↑↓、数字键。',
                'keys': [[[31], '继续']]},
    },
    'bind': {'touch': {'mode': 'swipe'}},
})
TD_RULES = [
    '## 怎么玩',
    '僵尸从右边沿三条路走过来，有一只走到最左边就输（「Zombies eat Your Brain!」）。这一关的僵尸全部出完、场上也清空，就赢（「You DEFEAT the Zombies!」）。',
    '方向键挪光标（16×16 的方框），F1 / F2 / F3 在光标那格种豌豆射手 / 太阳花 / 坚果墙。',
    '左上角 θ× 后面是阳光数。最下面一排是三种植物：左边的数是要花的阳光，右边的数是冷却，降到 0 才能再种这一种。',
    '## 植物',
    '豌豆射手：4 阳光，3 点血，一直往右发豌豆，一颗打掉僵尸 1 点血。',
    '太阳花：2 阳光，2 点血，隔几拍给 1 点阳光。',
    '坚果墙：2 阳光，8 点血，只挡路，冷却最长。',
    '僵尸碰到植物就停下来啃，一下 1 点血，啃光了植物就没了。',
    '## 僵尸',
    '普通僵尸 6 点血。铁桶僵尸 12 点血，铁桶打掉后变成普通僵尸再来 6 点。撑杆跳僵尸 6 点血，碰到植物、后面那格又空着，就跳过去，落地变普通僵尸。',
    '## 关卡（New Game 先选）',
    '1 Zombie Appears、2 Stronger Waves、3 A Big Fight：开局 4 点阳光，僵尸按写好的顺序出，中间会提示「A Huge Wave Of ZOMBIES Is APPOACHING」。',
    '4 !Zombie Sea!：开局 999 点阳光，先来 10 只，再随机来 60 只。',
    '## 暂停、继续',
    '游戏里按 MENU 暂停：Continue 接着玩，Quit 退出。退出后再运行，标题选 Continue 能接着上次的局面（计算器内存里的东西还在；刷新网页就没了）。',
    '标题的 Help 是一句玩笑：「什么都别种，看着僵尸走进你家就赢了」——照做会输。']
TD_CTL = {
    'controls': {
        'run': {'title': '塔防', 'text': '标题：↑↓ 选、EXE 确定。游戏里：方向键挪光标，F1 / F2 / F3 种豌豆射手 / 太阳花 / 坚果墙，MENU 暂停。作者原话：「因为平台和个人编码能力的问题，本游戏有点卡，请见谅。」嫌慢可以把速度调到 ×2 / ×4。',
                'pc': '电脑：方向键、Enter；F1–F3 或数字键 1–3 种植物；M = MENU 暂停。',
                'keys': [[[28], '上'], [[37], '下'], [[38], '左'], [[27], '右'], [[79], '豌豆'], [[69], '太阳花'], [[59], '坚果'], [[48], '暂停'], [[31], '确定']]},
    },
    'bind': {'keyboard': {'KeyM': 48, 'Digit1': 79, 'Digit2': 69, 'Digit3': 59, 'Numpad1': 79, 'Numpad2': 69, 'Numpad3': 59}, 'touch': {'mode': 'swipe'}},
    'rules': TD_RULES,
}
PROGRAMS.append({
    'file': 'TDGAME.G1M', 'size': 10144, 'entry': 'TDGAME', 'tag': '1.0', 'title': '塔防（植物大战僵尸，1.0）', 'date': '2010-10-22',
    'about': '「【.G1M】【9750可用】塔防小游戏」：用 DrawStat 拿列表画矢量小图，Pict 存背景。说明书原话：「最下面一排：[F1~F3]种下植物最下面一排对应的植物，从左往右分别是：{豌豆射手hp3,太阳花hp2,坚果墙hp8} 植物下面的数字，左边是需要的阳光数，右边是冷却时间，到0之后才可以种植物。左上角是你拥有的阳光数。」cnCalc 5009，2010-10-22 22:29。',
    'notes': [], **TD_CTL})
PROGRAMS.append({
    'file': 'TDGAME11.G1M', 'size': 11036, 'entry': 'TDGAME', 'tag': '1.1', 'title': '塔防（植物大战僵尸，1.1）', 'date': '2010-10-24',
    'about': '两天后的 1.1 版。更新说明原话：「@去除了胜利时显示43的16进制的BUG。@去除了撑杆跳僵尸跳过子弹的BUG。@太阳花生产阳光的速度适当减慢了。」cnCalc 5009，2010-10-24 10:55。',
    'notes': [], **TD_CTL})

# ---- C 写的 Add-in（G1A）：源码在 dir（相对仓根：originals/ = 当年的原件，2026/ = 2026 版），build_g1a.py 编成 WebAssembly ----
# 规则同上：只写游戏规则，从代码里读出来的。
ADDINS = [{
    'kind': 'g1a', 'addin': 'MAGTOWER', 'entry': 'MAGTOWER', 'file': 'MAGTOWER.G1A', 'dir': '20120703 魔塔/originals/src', 'title': '魔塔（MAGTOWER）', 'date': '2012-08-07',
    'about': '经典「魔塔」在 9860 上的移植，C 写的 Add-in。2012-07-05 在论坛挖坑（7616 帖：「还差战斗模块没写，地图码了6F/20多F」），'
             '08-07 15:24 发布（7735 帖），中文对话用 12×12 点阵字库 HZK12.df（和游戏一起发的那份），能存档。'
             '这里跑的是同一天贴出来的源码（code.rar，MagTower.c 文件时间 08-07 15:34），一个字没改，编成 WebAssembly。'
             '08-09 更新过一版 G1A（走路变快、战斗加快、加了 [shift] 看地图、修了几个 bug），那版只有二进制、没有源码，这里没有包括。',
    'notesTitle': '原帖原话（7735 帖）',
    'notes': ['操作说明（08-09 版的，[shift] 看地图这里没有）：「上下左右移动 / [EXE]确认 / [exit]调出菜单 / 在对话时按[exit]可跳过对话 / [optn]怪物图鉴（拿到后才可以） / [+][-]楼层跳跃（拿到后才可以）」',
              '「按[不告诉你]+[不告诉你]+[不告诉你]可调出作弊器，注意不要内存溢出」（组合键写在折叠的「规则和攻略」里）',
              '「说明一下，没有第22层，因为我自己打不上去。如果有谁能上去麻烦贴张截图，谢谢」',
              '7616 帖：「开不开源无所谓，其实本工程最难的是这个」（黑白图块）「画了两个礼拜（断断续续的，权当做调剂，所以实际没那么恐怖）」',
              '发布第二天网友 81100118 报了一串 bug（金币和经验、先扣血的怪、19 层的商人、没有结局……），08-09 版修了「几个」；这里是 08-07 的源码，这些都原样在。'],
    'saves': ['MagTower.sav'], 'saveSizes': {'MagTower.sav': 2742},  # = sizeof(player) 80 + sizeof(map) 22×11×11（save() 原样写这两块）
    'saveNote': '存档存在这个浏览器里（相当于计算器的存储内存）：换浏览器、清网站数据就没了；iPad 上的浏览器长时间不打开这个页面，系统也可能把它清掉。「导出存档」能存一份文件备着，换设备时「导入存档」。',
    'rules': [
        '## 勇者',
        '开局生命 1000、攻击 10、防御 10，从 0 层出发。',
        '升一级 = 生命 +1000、攻击 +10、防御 +10。',
        '## 战斗',
        '走到怪物身上就开打，自动回合，你先砍：你每下打掉「你的攻击 − 怪的防御」，怪每下打掉你「怪的攻击 − 你的防御」（不够减算 0）。',
        '开打前先估算：打不动（攻击不超过怪的防御），或者估计要掉的血不少于现在的生命，就不让打（「你打不过！」）。',
        '有几种怪开打前先扣你的血：初级巫师 100、高级巫师 300、灵法师扣现有生命的 1/3、白衣魔王扣 1/4。',
        '打赢拿金币和经验。',
        '## 门和钥匙',
        '钥匙分 A、B、C 三种，开对应的门各用一把；有一种门不用钥匙。',
        '## 道具（走上去就拿）',
        '盾：铁盾 / 银盾 / 骑士盾 / 圣盾 / 星光神盾，防御 +10 / 30 / 70 / 120 / 150。剑：铁剑 / 银剑 / 骑士剑 / 圣剑 / 星光神剑，攻击 +10 / 30 / 70 / 120 / 150。',
        '攻击宝石 攻击 +3，防御宝石 防御 +3；小血瓶 生命 +200，大血瓶 +500，圣水 生命翻倍；大金币 金币 +300。',
        '小飞羽 升 1 级；大飞羽 写的是「提升3级」，实际也是升 1 级。',
        '铁锒头、黄金十字架是剧情道具；怪物图鉴拿到后按 OPTN 看每种怪的数值和打它要掉多少血；万能钥匙 A、B、C 各 +1；电梯拿到后站在楼梯口按 + / − 去去过的楼层。',
        '## 商店和人',
        '3 层商店：25 金币换 攻击 +4 / 防御 +4 / 生命 +800 之一；11 层商店：100 金币换 攻击 +20 / 防御 +20 / 生命 +4000 之一。',
        '5 层、13 层的老人用经验换：5 层 30 经验换攻击或防御 +5、100 经验升 1 级；13 层 90 经验换 +18、270 经验升 3 级。',
        '5 层商人卖钥匙（A / B / C = 10 / 50 / 100 金币），12 层商人收钥匙（7 / 35 / 70 金币）。',
        '2 层的商人和老人各送一样东西；15 层的商人 500 金币、老人 500 经验各换一样东西。',
        '## 存档',
        'EXIT（或 AC）叫出菜单：继续 / 保存 / 退出。开局选「继续」读档。',
        '按 MENU 回列表时，游戏中会先问要不要存档（和真机上切到别的应用一样）。',
        '## 作弊器（原帖：「按[不告诉你]+[不告诉你]+[不告诉你]可调出作弊器，注意不要内存溢出」）',
        'SHIFT + 7 + F6 同时按住（电脑上 Shift + 7 + F6）。菜单：升 10 级 / 金币和经验各 +1000 / 开关作弊模式。',
        '作弊模式下 + / − 直接换楼层（不用电梯、不用站楼梯口），没拿到图鉴也能按 OPTN 看。原帖：「作弊器偷懒没加if可以到-1层，然后-2层爆掉」。'],
    'controls': {'run': {
        'title': '魔塔',
        'text': '方向键走路（按住一直走）；EXE 确定、翻对话；EXIT 叫出菜单（继续 / 保存 / 退出），对话时按 EXIT 跳过；拿到之后 OPTN 看怪物图鉴、+ / − 坐电梯。菜单里也能按 1–4 直接选。触屏：在屏幕上滑 = 走一步，点屏幕 = EXE。',
        'pc': '电脑：方向键或 WASD 走，Enter = EXE，Esc = EXIT，O = OPTN，+ / − = 电梯，M = MENU。',
        'keys': [[[28], '上'], [[37], '下'], [[38], '左'], [[27], '右'], [[31], '确定'], [[47], '菜单'], [[34], '菜单'], [[68], '图鉴'],
                 [[42], '电梯上'], [[32], '电梯下'], [[72, 62, 52, 73], '选项']]}},
    'bind': {'keyboard': {**WASD, 'KeyO': 68, 'Equal': 42, 'NumpadAdd': 42, 'Minus': 32, 'NumpadSubtract': 32, 'KeyM': 48,
                          'ShiftLeft': 78, 'ShiftRight': 78},
             'touch': {'mode': 'swipe'}},
    'fls0': {'HZK12.df': '20120703 魔塔/originals/HZK12.df'}, 'source': ['MagTower.c', 'pictures.h', 'dialog.h', 'monsters.h', 'map.h'],  # 按 include 的顺序；当年 .h 当模块用（作者 10-31「我那时不会用link，所以只能用h来当模块」）
}, {
    'kind': 'g1a', 'addin': 'DOODLE', 'entry': 'DOODLE', 'file': 'DOODLE.G1A', 'dir': '20110415 Doodle Jump/originals/2013', 'title': 'Doodle Jump', 'date': '2013-07-23',
    'portrait': True, 'ghost': 0,  # 不加液晶残影（user 10-31「doodle的残影蛮难受，而且感觉是网页端渲染的问题」）
    'about': 'Doodle Jump 的 9860 版，C 写的 Add-in。计算器要逆时针转过来竖着拿（屏幕右边朝上），这里直接竖着显示。'
             '论坛 5894 帖 2011-04-15 发的是 V0.9；这里跑的是作者电脑里留下的 2013-07-23 的源码（同一天编的 DOODLE.G1A 也在），比论坛那版新：'
             '修了「伪随机必死空隙」——生成假平台时累计一段没有能踩的高度，不让它超过最大跳跃高度。平台有 4 种。'
             '加了怪物的版本作者做过，没保存下来，留下来的只有这一版。',  # user 10-31「怪物我做了，应该就是没保存下来」（公开页面不引 2026 年的聊天原话）
    'notesTitle': '原帖原话（5894 帖）',
    'notes': ['2011-04-15 21:45 发帖：「Doodle Jump V0.9」',
              '2011-04-17：「我错了，一是随机数木有初始化，二是没有认真测试，10：00传上去后就睡觉第二天奔莫干山去了…… 不喜欢加开始界面，我有点趋向于编程复杂度换空间复杂度。1.5M有点小小。 我还有点事，下礼拜：不再是随机关卡；怪物，竹蜻蜓，火箭。」',
              '2013 年这份源码里随机数也没有初始化：每一局的关卡都一样（真机上每次重启后也一样）。'],
    'rules': [
        '一直往上跳：落到平台上自动弹起来，越爬越高。',
        '左右移动；松开就停。从屏幕一边出去会从另一边进来。',
        '平台：实线的是普通平台；中间断开的是假平台，一踩就碎，接着往下掉；中间多一道横线的会左右移动。',
        '分数 = 爬过的高度，写在屏幕上方。过 1500、3000、4500、6000、9000、12000 分各变难一次：平台变稀、移动平台变快、假平台和移动平台变多。',
        '掉出屏幕下边就结束；按 EXIT 也结束。结束后按任意键（真机上这一步会重启计算器，这里回列表）。'],
    'controls': {'run': {
        'title': 'Doodle Jump',
        'text': '屏幕是竖着的。左右移动用计算器的 ↑ ↓ 键（计算器转过来之后它们就在左右）。触屏：按住屏幕左半边往左、右半边往右。想用重力感应玩 Doodle Jump 2026。EXIT 结束；结束后按任意键回列表。',
        'pc': '电脑：← → 或 A / D。',
        'keys': [[[28], '左'], [[37], '右'], [[47], '结束']]}},
    'bind': {'keyboard': {'ArrowLeft': 28, 'ArrowRight': 37, 'KeyA': 28, 'KeyD': 37},
             'touch': {'mode': 'halves', 'left': 28, 'right': 37},
             },  # 原样版不接重力感应（user 10-31「doodle原版不要重力了」）；2026 版才有  # 重力感应：往左倾 = 按住左键（user 10-31「当然应该接ios的重力传感器」）
    'fls0': {}, 'source': ['Doodle.c'],
}, {
    'kind': 'g1a', 'addin': 'DOODLE2026', 'entry': 'DOODLE 2026', 'file': 'DOODLE.G1A（2026）', 'dir': '20261031 Doodle Jump 2026', 'title': 'Doodle Jump 2026', 'date': '2013-07-23',
    'portrait': True, 'ghost': 0, 'fullscreen': True,  # 开始画面点第一下就进全屏（user 10-31「然后进去就是可以直接全屏的」）
    'about': '以上面原样版的源码为底重写了几处，不是默认版本。'
             '一、跟屏幕刷新走、按经过的时间算位移（锁帧算 dt）：原样版是 25 毫秒一跳，40 帧的画面放在 60 帧的屏幕上一步长一步短，这一版不会。'
             '二、重力感应按比例：倾 15° 是原版的最快速度，倾得更多还能更快，到 30° 是两倍、不再加（原样版只认倾没倾）。'
             '三、随机数初始化了，每局关卡不一样（2011 年原帖里作者自己说「随机数木有初始化」是个错）。'
             '四、手感参数可以在下面调。五、开始画面和触屏：点一下开始（同时进全屏、申请重力感应），掉下去后点一下直接再来。'
             '六、跟手：没开重力感应时按住屏幕（电脑上按住鼠标），小人往手指那一列追，有惯性（先试最简单的二值控制：手指在哪边就往哪边全力加速，所以会在手指两边晃几下）。'
             '画面、平台、难度都和原样版一样。',
    'notes': [],
    'rules': [
        '一直往上跳：落到平台上自动弹起来，越爬越高。',
        '左右移动：开了重力感应就把手机 / iPad 往左右倾，倾得越多走得越快（倾 15° 是原版的最快，30° 是两倍）；没开就按住屏幕（电脑上按住鼠标），小人往那一列追；也可以用 ↑ ↓ 键，按住加速、松开就停。从屏幕一边出去会从另一边进来。',
        '平台：实线的是普通平台；中间断开的是假平台，一踩就碎；中间多一道横线的会左右移动。',
        '分数 = 爬过的高度。过 1500、3000、4500、6000、9000、12000 分各变难一次。',
        '掉出屏幕下边就结束，点一下（或按 EXE）直接再来一局，每局关卡不一样。EXIT 回列表。'],
    'controls': {'run': {
        'title': 'Doodle Jump 2026',
        'text': '开始画面点一下屏幕：进全屏、打开重力感应，把手机 / iPad 往左右倾（倾 15° 是原版最快，30° 两倍）。没有重力感应就按住屏幕，小人往手指那一列追；也可以用计算器的 ↑ ↓ 键。掉下去后点一下再来。',
        'pc': '电脑：按住鼠标，小人往鼠标那一列追；或 ← → / A D。',
        'keys': [[[28], '左'], [[37], '右'], [[31], '开始'], [[47], '回列表']]}},
    'bind': {'keyboard': {'ArrowLeft': 28, 'ArrowRight': 37, 'KeyA': 28, 'KeyD': 37},
             'touch': {'mode': 'game'}, 'sensor': True},  # 液晶上的手指原样交给游戏（Touch_Read / Touch_Hit）
    # 调手感（user 10-31「待会你把数据给我一下，我来调手感」）：编号和 Doodle2026.c 的 P_* 一致；单位按这一行的说法（像素、度）
    'params': [
        {'id': 6, 'name': '倾多少度 = 原版最快', 'unit': '0.01°', 'def': 1500},  # user 10-31「15度原版量程，30度两倍封顶」
        {'id': 7, 'name': '最快封顶（原版最快的几倍）', 'unit': '%', 'def': 200},
        {'id': 5, 'name': '倾角死区', 'unit': '0.01°', 'def': 0},
        {'id': 1, 'name': '原版横向最快速度', 'unit': '像素/秒', 'def': 80},
        {'id': 2, 'name': '键盘横向加速度', 'unit': '像素/秒²', 'def': 800},
        {'id': 8, 'name': '跟手加速度（二值控制，全力）', 'unit': '像素/秒²', 'def': 800},  # user 10-31「惯性应该还是要有的，所以我才说先二值控制，之后再尝试比例和pid」
        {'id': 9, 'name': '跟手最快速度', 'unit': '像素/秒', 'def': 80},
        {'id': 3, 'name': '起跳速度', 'unit': '像素/秒', 'def': 220},
        {'id': 4, 'name': '重力', 'unit': '像素/秒²', 'def': 320}],
    'fls0': {}, 'source': ['Doodle2026.c'], 'sourceNote': '豪华加强版，以当年的 Doodle.c 为底',
}, {
    'kind': 'g1a', 'addin': 'ENGINE3D', 'entry': '3DENGINE', 'file': '3DENGINE.G1A', 'dir': '20111111 3D 引擎/originals', 'title': '3D 引擎（寒假前的初赛版）', 'date': '2012-02-03',
    'frameMs': 110,  # 论文 4.2.2「普通测试」原生 29.5 MHz 下 9.1 帧/秒 → 每帧 110 毫秒；×2 = 论文演示用的 2 倍超频
    'about': 'C 写的第一人称 3D 引擎：贴图的墙、地板和物件，画家算法加一维集合算法做消隐（后来写成论文《一个简单的计算机3D图形引擎及其在CASIO 9750GII图形计算器上的实现》，2012-08-12）。'
             '这里跑的是 cnCalc 7079 帖 2012-02-05 贴的源码 3Dold.rar（3DEngine.c 文件时间 2012-02-03）。'
             '速度：论文 8 月版在原生 29.5 MHz 下「普通测试」9.1 帧/秒，演示用 2 倍超频；这里速度 ×1 固定 9.1 帧/秒、×2 = 2 倍超频。'
             '这是估的：真机帧率随场景变，而且这份 2 月的版本比论文那版慢（见下面原话）。'
             '画面对过：开局第一帧和 7079 帖同时贴的截图 123.bmp 逐像素一致（8064 个点全对，第 1 行的调试点除外）。',
    'notesTitle': '原帖原话（7079 帖，2012-02-05）',
    'notes': ['「一个3D的源码，凑合着看一下，还没写好」（帖子标题）',
              'diameter：「跑起来的时候肿么这么慢..」',
              'GWHBOB：「呜哇，还没写完就开学了……这是寒假之前编的应付初赛的版本。卡的原因是地图太大，需要优化。 目前的版本已经快很多了，但脚手架还没拆，所以不能发上来。——另外一个原因，这是去刷创新大赛的，要是赛还没比就网上乱丢，另外一名合作的同学还不把我吃掉~~」',
              'NAT：「汗....怎么能穿墙?」'],
    'rules': [
        '没有目标：在一个小场景里走走看看（演示版）。',
        '屏幕最上面一行的几个点是调试用的：帧计数、朝向、这一帧排进渲染队列的东西有几个。',
        '走路时手里的东西会上下晃。',
        'AC：真机上是重启计算器（Reset_Calc），这里回列表。'],
    'controls': {'run': {
        'title': '3D 引擎',
        'text': '数字键 8 / 5 前进后退，4 / 6 左右平移；← → 转身，↑ ↓ 抬头低头；+ / − 调视野缩放；AC 退出。速度 ×1 = 原生 29.5 MHz，×2 = 论文演示用的 2 倍超频。',
        'pc': '电脑：WASD 走，← → 转身，↑ ↓ 抬头低头，+ / − 缩放，Esc 退出。',
        'keys': [[[64], '前进'], [[63], '后退'], [[73], '左移'], [[53], '右移'], [[38], '左转'], [[27], '右转'], [[28], '抬头'], [[37], '低头'],
                 [[42], '放大'], [[32], '缩小'], [[34], '退出']]}},
    'bind': {'keyboard': {'KeyW': 64, 'KeyS': 63, 'KeyA': 73, 'KeyD': 53, 'Equal': 42, 'NumpadAdd': 42, 'Minus': 32, 'NumpadSubtract': 32, 'Escape': 34},
             'touch': {'mode': 'swipe'}},
    'fls0': {}, 'source': ['3DEngine.c', 'Textures.h', 'Maps.h', 'Public.h', 'Painter.h', 'DataStruct.h', 'PainterSort.h', 'Game_Script.h'],  # 按 include 的顺序；引擎本体在 .h 里
}]

# 魔塔 2026 版（考古家规（见 CLAUDE.md）：原样版是默认，2026 版另算、不当默认，两个版本共用一份存档；user 10-31「2026版就是修复版，没必要做三个版本」）
ADDINS.insert(1, {**ADDINS[0], 'addin': 'MAGTOWER2026', 'entry': 'MAGTOWER 2026', 'file': 'MAGTOWER.G1A（2026）', 'title': '魔塔 2026',
    'about': '现在和上面的原样版是同一份 08-07 源码，只补回当年转换图块时丢掉的 103 个字节（53 个图块里缺的点），别的一个字没动。'
             '缺点的原因：当年用自己写的 VB 小工具 MapEdit 把图片转成 C 数组，那一行是 "0x" & Format(Hex(b), "00")；'
             'Hex 给的是字符串，Format 想把它当数字，VB 却把 "1A"–"9A" 读成了凌晨 1–9 点（"2A" = 2 AM），格式化出来是 "00"，'
             '所以这 9 种值全成了 0。补丁照 MapEdit 同一套算法从当年的原图（魔塔黑白 - 副本.bmp，2012-07-05）重算。'
             '以后作者重画的图块也进这一版。不是默认版本；存档和原样版是同一份。',
    'notes': [], 'notesTitle': '', 'source': [], 'fls0': {},  # 字库已在存储内存里（原样版带进来的），不再嵌一份
    'saveNote': '存档和上面的原样版是同一份（\\fls0\\MagTower.sav），两边都能读、都能存。'})

progs = []
for p in PROGRAMS:
    b = open(os.path.join(root, BASIC_HOME[p['file']], 'originals', p['file']), 'rb').read()
    assert len(b) == p['size'], (p['file'], len(b))
    progs.append({**{k: v for k, v in p.items() if k != 'size'}, 'b64': base64.b64encode(b).decode()})
sys.path.insert(0, here)
import build_g1a  # noqa: E402
built = build_g1a.build_all()
addin_js = []


def src_text(path):  # 当年的源码是 GBK；2026 年新写的是 UTF-8
    b = open(path, 'rb').read()
    try:
        t = b.decode('utf-8')
    except UnicodeDecodeError:
        t = b.decode('gbk')
    return t.replace('\r\n', '\n')


for a in ADDINS:
    d = os.path.join(root, a['dir'])
    js = open(built[a['addin']], encoding='utf-8').read()
    assert '</script' not in js.lower()
    addin_js.append(js)
    progs.append({**{k: v for k, v in a.items() if k not in ('dir', 'fls0', 'source')},
                  'fls0': {n: base64.b64encode(open(os.path.join(root, f), 'rb').read()).decode() for n, f in a['fls0'].items()},
                  'source': {n: src_text(os.path.join(d, n)) for n in a['source']}})
# 帮助弹窗里的一句话简介（作者 10-31「全屏版需要有一个html的帮助按钮显示键位和简单简介」）；照各程序的说明和规则写
BRIEF = {'SNACKI.G1M': '贪吃蛇：吃到点就变长，撞墙或撞到自己就结束。开局先输入速度 1–10，越大越快。',
         '21.G1M': '21 点：最多 6 个人轮流用同一台计算器玩，可以加电脑玩家 CASIO；牌点加起来最接近 21 又不爆的赢。',
         'CRPG.G1M': '三国杀简版：你对电脑 ZZM，一人 5 点血，只有杀、闪、桃三种牌。',
         'TDGAME.G1M': '塔防（植物大战僵尸）：用 F1–F3 种豌豆射手、太阳花、坚果墙，挡住来的僵尸；左上角是阳光数。',
         'TDGAME11.G1M': '塔防（植物大战僵尸）1.1：两天后的更新版，修了几个 bug、太阳花产阳光慢了一点。F1–F3 种植物，挡住僵尸。',
         'MINE.G1M': '扫雷：开局输入雷数（作者建议 20~40），雷区 21 × 7；挖到空白会把周围一片翻开。',
         'MINEA.G1M': '扫雷（9750 版）：和 9860 版同一个程序，没翻开的格子用没刷机的 9750 也能显示的字符。',
         'MAGTOWER': '魔塔：经典的数值迷宫 RPG，打怪、捡钥匙开门、一层层往上爬；能存档。',
         'MAGTOWER2026': '魔塔 2026：和原样版同一份源码，补回当年转换图块时丢掉的点；和原样版共用存档。',
         'DOODLE': 'Doodle Jump：一直往上跳，落到平台上自动弹起，越爬越高，掉下去就结束。计算器要竖着拿。',
         'DOODLE2026': 'Doodle Jump 2026：锁帧更顺、重力感应按倾角比例、触屏、每局关卡不一样。',
         'ENGINE3D': '3D 引擎：2012 年寒假前的演示版，没有目标，在一个小场景里走走看看。'}
for p in progs: p['brief'] = BRIEF[p.get('addin') or p['file']]
font = json.load(open(os.path.join(here, 'casio-font.json'), encoding='utf-8'))
shell = open(os.path.join(here, 'shell.html'), encoding='utf-8').read()
small = open(os.path.join(here, 'casio-font-small.json'), encoding='utf-8').read()
font_js = json.dumps(font, ensure_ascii=False, separators=(',', ':'))
key = lambda p: p.get('addin') or p['file']
def page(members, autorun, title):  # 一页只带这几个程序和它们的 wasm
    sub = [p for p in progs if key(p) in members]
    js = [j for a, j in zip(ADDINS, addin_js) if a['addin'] in members]
    h = shell.replace('%%ADDINS_JS%%', '\n'.join(js)).replace('%%FONT_SMALL%%', small).replace('%%AUTORUN%%', str(autorun)).replace('%%START_FS%%', 'false')
    h = h.replace('%%FONT%%', font_js).replace('%%PROGRAMS%%', json.dumps(sub, ensure_ascii=False))
    return h.replace('<title>GWHBOB的计算器程序</title>', f'<title>{title}</title>') if title else h
html = page({key(p) for p in progs}, -1, None)
open(os.path.join(here, 'index.html'), 'w', encoding='utf-8').write(html)
print('index.html', len(html.encode()), 'bytes,', len(progs), 'files')
# 每个游戏一个网页，打开就玩（作者 10-31「我建议是不同游戏每个一个网页？这样我可以分享单个游戏。然后每个网页都优先是点开能玩」
# 「我觉得没必要要那个计算器内部导航，反而麻烦」）：一页一个程序，网址用短英文名好分享；原样版和 2026 版各一页。
PAGES = [('snake', 'SnackI 贪吃蛇', 'SNACKI.G1M'), ('21', '21 点', '21.G1M'), ('sanguosha', '三国杀简版', 'CRPG.G1M'),
         ('td', '塔防 1.1', 'TDGAME11.G1M'), ('td-1.0', '塔防 1.0', 'TDGAME.G1M'),
         ('mine', '扫雷（9860 版）', 'MINE.G1M'), ('mine-9750', '扫雷（9750 版）', 'MINEA.G1M'),
         ('doodle', 'Doodle Jump', 'DOODLE'), ('doodle-2026', 'Doodle Jump 2026', 'DOODLE2026'),
         ('3d', '3D 引擎', 'ENGINE3D'), ('magtower', '魔塔', 'MAGTOWER'), ('magtower-2026', '魔塔 2026', 'MAGTOWER2026')]
by_key = {key(p): p for p in progs}
os.makedirs(os.path.join(here, 'play'), exist_ok=True)
for slug, title, k in PAGES:
    p = dict(by_key[k])
    if k == 'MAGTOWER2026': p['fls0'] = by_key['MAGTOWER']['fls0']   # 合集页里字库是原样版带进存储内存的；单独一页要自己带
    js = [j for a, j in zip(ADDINS, addin_js) if a['addin'] == k]
    h = shell.replace('%%ADDINS_JS%%', '\n'.join(js)).replace('%%FONT_SMALL%%', small).replace('%%AUTORUN%%', '0').replace('%%START_FS%%', 'true')
    h = h.replace('%%FONT%%', font_js).replace('%%PROGRAMS%%', json.dumps([p], ensure_ascii=False)).replace('<title>GWHBOB的计算器程序</title>', f'<title>{title}</title>')
    open(os.path.join(here, 'play', slug + '.html'), 'w', encoding='utf-8').write(h)
    print(f'play/{slug}.html', len(h.encode()), 'bytes')

# 合集页（home.html → 宣发仓的 index.html）：1-bit 液晶风格，每个展位 = 游戏 + 源码 + 介绍。中文只带页面上用到的字（HZK12 子集）。
REPO = 'https://github.com/fangzhangmnm/casio-2010'
from urllib.parse import quote
# 每个展厅（子仓）一页「源码化石」：站内的 src/<短名>.html（作者 10-31「源码为什么跳转github，不是github源码，而是策展式的源码化石」）
HALLS = [('snake', '20100829 贪吃蛇', '贪吃蛇'), ('21', '20100829 21点', '21点'), ('sanguosha', '20100918 三国杀简版', '三国杀简版'),
         ('td', '20101022 塔防', '塔防'), ('doodle', '20110415 Doodle Jump', 'Doodle Jump'), ('3d', '20111111 3D 引擎', '3D 引擎'),
         ('mine', '20120629 扫雷', '扫雷'), ('magtower', '20120703 魔塔', '魔塔'), ('pinyin', '20120813 拼音输入法', '拼音输入法'),
         ('ogame', '20130212 Ogame', 'Ogame'), ('visualfx', '20130214 VisualFX', 'VisualFX'), ('danmaku', '20130411 弹幕骨架', '弹幕骨架'),
]
HALL_SRC = {h: slug for slug, h, _ in HALLS}
tree = lambda hall: f"src/{HALL_SRC[hall]}.html" if hall in HALL_SRC else None   # 历史作品 = 化石页；2026 版不放
SECTIONS = [
    ('豪华加强版', [('2026-10-31', '魔塔 2026', 'magtower-2026', '20261031 魔塔 2026'), ('2026-10-31', 'Doodle Jump 2026', 'doodle-2026', '20261031 Doodle Jump 2026')]),   # 作者 10-31 定名「豪华加强版」：没有语义、纯谜语人，往里放什么都行   # 放最上（作者 10-31「豪华重制版放上面，historical放下面」）
    ('能玩的', [('2010-08-29', '贪吃蛇', 'snake', '20100829 贪吃蛇'), ('2010-08-29', '21点', '21', '20100829 21点'),
               ('2010-09-18', '三国杀简版', 'sanguosha', '20100918 三国杀简版'), ('2010-10-22', '塔防 1.0', 'td-1.0', '20101022 塔防'),
               ('2010-10-24', '塔防 1.1', 'td', '20101022 塔防'), ('2012-02-03', '3D引擎', '3d', '20111111 3D 引擎'),
               ('2012-06-29', '扫雷 9860版', 'mine', '20120629 扫雷'), ('2012-06-29', '扫雷 9750版', 'mine-9750', '20120629 扫雷'),
               ('2012-08-07', '魔塔', 'magtower', '20120703 魔塔'), ('2013-07-23', 'Doodle Jump', 'doodle', '20110415 Doodle Jump')]),
    ('还没复活（只有化石）', [('2012-08-13', '拼音输入法', None, '20120813 拼音输入法'), ('2013-02-12', 'Ogame', None, '20130212 Ogame'),
                     ('2013-02-14', 'VisualFX', None, '20130214 VisualFX'), ('2013-04-11', '弹幕骨架', None, '20130411 弹幕骨架')]),
]
assert all(slug in {p[0] for p in PAGES} for _, rows in SECTIONS for _, _, slug, _ in rows if slug)
home = {'title': 'GWHBOB 的计算器程序', 'sub': 'fx-9750GII  2010-2013', 'repo': REPO,
        'sections': [{'name': n, 'rows': [{'date': d, 'name': t, 'play': (f'play/{slug}.html' if slug else None), 'src': tree(h)} for d, t, slug, h in rows]} for n, rows in SECTIONS]}
# 计算器 / 化石 / 介绍改成像素小图标，不再用液晶字
texts = [home['title'], home['sub']] + [s['name'] for s in home['sections']] + [r['date'] + ' ' + r['name'] for s in home['sections'] for r in s['rows']]
hzk = open(os.path.join(root, '20120703 魔塔', 'originals', 'HZK12.df'), 'rb').read()
cjk = {}
for ch in sorted({c for t in texts for c in t if ord(c) > 127}):
    b = ch.encode('gb2312'); i = 94 * (b[0] - 0xA1) + (b[1] - 0xA1)
    cjk[ch] = hzk[i*24:(i+1)*24].hex()
asc = {f'{ord(c):X}': font[f'{ord(c):X}'] for c in sorted({c for t in texts for c in t if 32 < ord(c) < 128}) if f'{ord(c):X}' in font}  # 字库的键是大写十六进制
home['cjk'] = cjk; home['ascii'] = asc
missing = sorted({c for t in texts for c in t if 32 < ord(c) < 128} - {chr(int(k, 16)) for k in asc})
assert not missing, missing
h = open(os.path.join(here, 'home.html'), encoding='utf-8').read().replace('%%HOME_DATA%%', json.dumps(home, ensure_ascii=False))
os.makedirs(os.path.join(here, 'site'), exist_ok=True)
open(os.path.join(here, 'site', 'index.html'), 'w', encoding='utf-8').write(h)
print('site/index.html', len(h.encode()), 'bytes;', len(cjk), '个中文字（HZK12 子集）')

# ---- 源码化石页：site/src/<短名>.html。原件的下载链接指向宣发仓里的展厅（../<子仓>/…），所以要和宣发仓的布局一起看 ----
import html as H, re, subprocess
TEXT = {'c', 'h', 'txt', 'frm', 'bat', 'g1w', 'py', 'json'}
IMG = {'bmp', 'jpg', 'jpeg', 'png', 'gif'}
PLAY_OF = {}
for slug, title, k in PAGES:
    for d, t, ps, hall in [r for _, rows in SECTIONS for r in rows]:
        if ps == slug: PLAY_OF.setdefault(hall, []).append((slug, title))
def manifest(hall):
    out = {}
    for line in open(os.path.join(root, hall, 'MANIFEST.md'), encoding='utf-8') if os.path.exists(os.path.join(root, hall, 'MANIFEST.md')) else []:
        m = re.match(r'\| `([^`]+)` \| ([^|]*)\| ([^|]*)\| `?([^|`]*)`? \| (.*) \|\s*$', line)
        if m: out[m.group(1)] = [x.strip() for x in m.groups()[1:]]
    return out
def md_inline(t): return re.sub(r'`([^`]+)`', r'<code>\1</code>', H.escape(t))
def label(hall):                                     # 子仓的 README 当展品标签（极简 markdown）
    out = []
    for line in open(os.path.join(root, hall, 'README.md'), encoding='utf-8').read().split('\n'):
        if line.startswith('# ') or not line.strip(): continue
        if line.startswith('> '): continue                      # 署名、来源这些给 agent 看的，不上标签
        if '../' in line or 'MANIFEST' in line: continue       # 给开发看的路径、和下面原件表重复的，不上展品标签
        out.append(f'<li>{md_inline(line[2:])}</li>' if line.startswith('- ') else f'<p>{md_inline(line)}</p>')
    return '\n'.join(out).replace('</li>\n<li>', '</li><li>').replace('<li>', '<ul><li>', 1) + ('</ul>' if '<li>' in ''.join(out) else '')
def text_of(path):
    b = open(path, 'rb').read()
    try: return b.decode('utf-8')
    except UnicodeDecodeError: return b.decode('gbk', errors='replace')
SRC_CSS = """:root{--lcd:#c8ceba;--ink:#222620;--panel:#b9c0a9}html,body{margin:0;background:var(--lcd);color:var(--ink)}
main{max-width:860px;margin:0 auto;padding:16px;font:15px/1.6 system-ui,-apple-system,"PingFang SC",sans-serif}
a{color:var(--ink)}h1{font-size:22px;margin:8px 0}h2{font-size:17px;margin:20px 0 6px;border-bottom:2px solid var(--ink)}
table{border-collapse:collapse;width:100%;font-size:13px}td,th{padding:3px 6px;border-bottom:1px solid #9aa08c;text-align:left;vertical-align:top}
td.n{text-align:right;white-space:nowrap}code,pre{font:13px/1.45 ui-monospace,Menlo,Consolas,"Noto Sans Mono CJK SC",monospace}
pre{background:var(--panel);padding:10px;overflow:auto;white-space:pre;tab-size:4}details{margin:6px 0}summary{cursor:pointer;font-weight:600}
.imgs{display:flex;flex-wrap:wrap;gap:12px}.imgs figure{margin:0}.imgs img{image-rendering:pixelated;max-width:100%;border:1px solid #9aa08c}
.imgs figcaption{font-size:12px}.top a{margin-right:12px}.wrap{overflow-x:auto}"""
os.makedirs(os.path.join(here, 'site', 'src'), exist_ok=True)
for slug, hall, name in HALLS:
    files = [f for f in subprocess.run(['git', '-c', 'core.quotePath=false', '-C', os.path.join(root, hall), 'ls-files', '-z'], capture_output=True, text=True, check=True).stdout.split('\0')
             if f and os.path.basename(f) not in ('.gitignore', 'README.md', 'MANIFEST.md')]
    man = manifest(hall); url = lambda f: '../' + quote(hall) + '/' + quote(f)
    ext = lambda f: f.rsplit('.', 1)[-1].lower() if '.' in f else ''
    rows = ''.join(f'<tr><td><a href="{url(f)}" download>{H.escape(f)}</a></td><td class="n">{H.escape(man.get(f, ["", "", "", ""])[0]) or os.path.getsize(os.path.join(root, hall, f))}</td>'
                   f'<td>{H.escape(man.get(f, ["", "—"])[1])}</td><td><code>{H.escape(man.get(f, ["", "", ""])[2])}</code></td><td>{md_inline(man.get(f, ["", "", "", "2026 年写的"])[3])}</td></tr>' for f in files)
    texts = [f for f in files if ext(f) in TEXT]
    main_src = next((f for f in texts if ext(f) == 'c'), None) or next((f for f in texts if f.startswith('listings/')), texts[0] if texts else None)
    codes = ''.join(f'<details{" open" if f == main_src else ""}><summary>{H.escape(f)}</summary><pre>{H.escape(text_of(os.path.join(root, hall, f)))}</pre></details>' for f in texts)
    imgs = ''.join(f'<figure><img src="{url(f)}" alt="{H.escape(f)}" loading="lazy"><figcaption>{H.escape(f)}</figcaption></figure>' for f in files if ext(f) in IMG)
    plays = ''.join(f'<a href="../play/{ps}.html">▶ 玩 {H.escape(t)}</a><a href="../play/{ps}.html?calc">计算器</a>' for ps, t in PLAY_OF.get(hall, []))
    page_html = (f'<!doctype html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">'
                 f'<title>{H.escape(name)} · 化石</title><style>{SRC_CSS}</style></head><body><main>'
                 f'<p class="top"><a href="../">← GWHBOB的计算器程序</a>{plays}</p><h1>{H.escape(name)} · 化石</h1>{label(hall)}'
                 f'<h2>原件</h2><p>当年的字节，一个没改；点文件名下载。时间是文件里存的原始修改时间（北京时间），「—」是论坛附件、看出处里的帖子日期。</p>'
                 f'<div class="wrap"><table><tr><th>文件</th><th>字节</th><th>原始时间</th><th>sha256 前 16 位</th><th>出处</th></tr>{rows}</table></div>'
                 + (f'<h2>代码</h2><p>当年是 GBK 编码，这里解出来显示。</p>{codes}' if codes else '')
                 + (f'<h2>图</h2><div class="imgs">{imgs}</div>' if imgs else '') + '</main></body></html>')
    open(os.path.join(here, 'site', 'src', slug + '.html'), 'w', encoding='utf-8').write(page_html)
print('site/src/', len(HALLS), '页化石')

