# -*- coding: UTF-8 -*-


import sys
import time

import sens_proc

sys.path.append("./build/lib.linux-x86_64-3.6/")

import acdetector

if __name__ == "__main__":

    with open("all.uniq", "r", encoding="utf-8") as dic_f:
        word_list = [l.strip() for l in dic_f if l.strip()]
    print(len(word_list))
    detector = acdetector.ACDetector()

    start_t = time.time()
    sens_proc.processing_init("all.uniq")
    end_t = time.time() - start_t
    print("pass: ", end_t)

    # 读取敏感词表
    start_t = time.time()

    detector.build_ac(word_list)

    end_t = time.time() - start_t
    print("pass: ", end_t)

    test_words = "太多的伤感情怀也许只局限于饲养基地 荧幕中的情节，主人公尝试着去用某种方式渐渐的很潇洒地释自杀指南怀那些自己经历的伤感。" + \
        "然后法.轮.功 我们的扮演的角色就是跟随着主人公的喜红客联盟 怒哀乐而过于牵强的把自己的情感也附加于银幕情节中，然后感动就流泪，" + \
        "难过就躺在某一个人的怀里尽情的阐述心扉或者手机卡复制器一个人一杯红酒一部电影在夜三.级.片 深人静的晚上，关上电话静静的发呆着。"

    start_t = time.time()
    result = sens_proc.sens_processing(test_words)
    end_t = time.time() - start_t
    print(result)
    for p in result:
        print("%s %d %d" % (test_words[p['head']:p['tail']+1], p['head'], p['tail']))
    print("pass: ", end_t)

    start_t = time.time()
    result = detector.processing(test_words)
    end_t = time.time() - start_t
    print(result)
    for p in result:
        print("%s %d %d" % (test_words[p['head']:p['tail']+1], p['head'], p['tail']))
    print("pass: ", end_t)

    print(detector.is_active())
    # print(detector.clear_ac())

    start_t = time.time()

    detector.build_ac(word_list)

    end_t = time.time() - start_t
    print("pass: ", end_t)

    start_t = time.time()
    result = detector.processing(test_words)
    end_t = time.time() - start_t
    print(result)
    for p in result:
        print("%s %d %d" % (test_words[p['head']:p['tail']+1], p['head'], p['tail']))
    print("pass: ", end_t)
