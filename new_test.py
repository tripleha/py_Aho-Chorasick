# -*- coding: UTF-8 -*-


import sys
import time

sys.path.append("./build/lib.linux-x86_64-3.6/")

import acdetector


class ACD:

    def __init__(self, word_list):
        self.detector = acdetector.ACDetector()
        self.detector.build_ac(word_list)

    def rebuild(self, word_list):
        print("into")
        self.detector.build_ac(word_list)
        print("out")
    
    def clear_ac(self):
        print(self.detector.is_active())
        self.detector.clear_ac()

    def process(self, src):
        result = self.detector.processing(src)
        return result


if __name__ == "__main__":
    
    with open("all.uniq", "r", encoding="utf-8") as dic_f:
        word_list = [l.strip() for l in dic_f if l.strip()]

    test_words = "太多的伤感情怀也许只局限于饲养基地 荧幕中的情节，主人公尝试着去用某种方式渐渐的很潇洒地释自杀指南怀那些自己经历的伤感。" + \
    "然后法.轮.功 我们的扮演的角色就是跟随着主人公的喜红客联盟 怒哀乐而过于牵强的把自己的情感也附加于银幕情节中，然后感动就流泪，" + \
    "难过就躺在某一个人的怀里尽情的阐述心扉或者手机卡复制器一个人一杯红酒一部电影在夜三.级.片 深人静的晚上，关上电话静静的发呆着。"

    det = ACD(word_list)
    # det = acdetector.ACDetector()
    # det.build_ac(word_list)
    start_t = time.time()
    for i in range(100):
        result = det.process(test_words)
        det.rebuild(word_list)
        print(i)
        # result = det.processing(test_words)
        # det.build_ac(word_list)

    det.clear_ac()
    end_t = time.time() - start_t
    for p in result:
        print("%s %d %d" % (test_words[p['head']:p['tail']+1], p['head'], p['tail']))
    print(end_t)
