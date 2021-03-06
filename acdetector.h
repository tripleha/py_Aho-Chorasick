// Copyright deepintell
// License
// Author: tripleha
// 包含AC自动机创建和基于AC自动机的多模式匹配方法函数，主要用于在句子中查找词语

#include <string>
#include <locale>
#include <vector>
#include <unordered_map>
#include <queue>

using std::make_pair;
using std::queue;
using std::string;
using std::u32string;
using std::unordered_map;
using std::vector;
using std::wstring_convert;

#ifndef _ACDETECTOR_H_
#define _ACDETECTOR_H_

// 编码转换准备工作，主要在UTF-8与UTF-32编码间的转换
// 在C++17中已经删除这种用法，在C++11中使用
template <class internT, class externT, class stateT>
struct ncodecvt : std::codecvt<internT, externT, stateT>
{
    ~ncodecvt() {}
};

static wstring_convert<ncodecvt<char32_t, char, std::mbstate_t>, char32_t> convert32;

// 算法实现

struct TrieTreeNode
{
    TrieTreeNode *fail;                              // 失配指针，AC自动机使用，在查询时指示若匹配失败将要进入的下一个节点
    unordered_map<char32_t, TrieTreeNode *> *childs; // 下一个节点的字典
    int pos;                                         // 属于第几个模式串的结尾，-1代表不是结尾字符
    int level;                                       // 在trie树中的层数
    char32_t chr;                                    // 指示当前节点所代表的字符，使用UTF-32，因为在UTF-8中汉字不止占用一个字符

    TrieTreeNode()
    {
        fail = nullptr;
        childs = new unordered_map<char32_t, TrieTreeNode *>();
        pos = -1;
        level = 0;
        chr = 0;
    }
    ~TrieTreeNode()
    {
        if (childs)
        {
            // delete STL容器不会自动调用指针类型成员的析构函数，所以要自己迭代调用
            for (unordered_map<char32_t, TrieTreeNode *>::iterator iter = childs->begin(); iter != childs->end(); iter++)
            {
                if (iter->second)
                {
                    delete iter->second;
                    iter->second = nullptr;
                }
            }
            delete childs;
            childs = nullptr;
        }
    }
};

// 用于储存结果三元组的结构体
struct TripleIntNode
{
    int first;
    int second;
    int third;

    TripleIntNode(int first, int second, int third)
    {
        this->first = first;
        this->second = second;
        this->third = third;
    }
};

// 建立字典树，供创建AC自动机前调用
void build_trie(TrieTreeNode *root, vector<string> *word_list)
{
    string tmp_word;
    u32string tmp_word32;
    int count = 0; // 记录当前读到了词表中的第几个词
    int level;
    TrieTreeNode *now_node;

    for (auto word_iter = word_list->cbegin(); word_iter != word_list->cend(); word_iter++)
    {
        now_node = root;
        level = 0;

        tmp_word = (*word_iter);
        if (tmp_word == "")
        {
            count++;
            continue;
        }
        tmp_word32 = convert32.from_bytes(tmp_word);
        for (auto &chr : tmp_word32)
        {
            auto iter = now_node->childs->find(chr);
            if (iter == now_node->childs->end())
            {
                // 不在子节点中则新建分支
                TrieTreeNode *new_node = new TrieTreeNode();
                new_node->chr = chr;
                new_node->level = level;
                now_node->childs->insert(make_pair(chr, new_node));
                now_node = new_node;
            }
            else
            {
                // 存在子节点中
                now_node = iter->second;
            }
            level++;
        }
        now_node->pos = count;
        count++;
    }
}

// 返回创建AC自动机的根节点地址
TrieTreeNode *create_ac(vector<string> *word_list)
{
    TrieTreeNode *root = NULL;
    root = new TrieTreeNode();
    // 创建AC自动机需要一个创建好的字典树
    build_trie(root, word_list);

    queue<TrieTreeNode *> tmp_q = queue<TrieTreeNode *>();
    root->fail = root;
    // 首先处理root的孩子，将它们的fail指针设为root
    for (unordered_map<char32_t, TrieTreeNode *>::iterator iter = root->childs->begin(); iter != root->childs->end(); iter++)
    {
        iter->second->fail = root;
        tmp_q.push(iter->second);
    }

    TrieTreeNode *pa_node;
    // 开始进行广度优先搜索
    while (!tmp_q.empty())
    {
        pa_node = tmp_q.front();
        tmp_q.pop();
        for (unordered_map<char32_t, TrieTreeNode *>::iterator iter = pa_node->childs->begin(); iter != pa_node->childs->end(); iter++)
        {
            tmp_q.push(iter->second);
            TrieTreeNode *tmp_node = pa_node;
            while (tmp_node != root)
            {
                tmp_node = tmp_node->fail;
                auto tmp_son = tmp_node->childs->find(iter->first);
                if (tmp_son != tmp_node->childs->end())
                {
                    // 找到了同字符的子节点，指定fail指针指向该节点
                    iter->second->fail = tmp_son->second;
                    break;
                }
            }
            if (!iter->second->fail)
            {
                // 未找到同字符节点，将fail指针指向root
                iter->second->fail = root;
            }
        }
    }

    return root;
}

// 返回传入句子中包含的ac自动机模式词语列表，只发现最长敏感词
void find_all_ac(TrieTreeNode *root, string *words, vector<TripleIntNode> *result)
{
    // 必须保证中文字符也是一长度才能进行匹配
    u32string word_str32 = convert32.from_bytes(*words);
    const char32_t *word32 = word_str32.c_str();
    int length = word_str32.length();

    TrieTreeNode *now_node = root;

    vector<int> tmp_begin_pos = vector<int>();
    vector<TrieTreeNode *> tmp_match = vector<TrieTreeNode *>();
    TrieTreeNode *last_match;
    bool is_put;
    int begin_pos;

    for (int i = 0; i < length; i++)
    {
        is_put = false;
        while (now_node != root)
        {
            // 若是匹配位置处于中间节点才会进入此循环
            auto iter = now_node->childs->find(word32[i]);
            if (iter == now_node->childs->end())
            {
                // 未找到匹配的子节点，移动到fail指针指向节点
                now_node = now_node->fail;
                // 当回溯失败指针发生时，说明当前字符匹配失败了
                // 那么之前如果存在匹配结果的话可以将其加入最终结果了
                // 故设立标志位
                is_put = true;
            }
            else
            {
                // 找到了匹配的子节点，移动到该子节点
                now_node = iter->second;
                break;
            }
        }
        if (is_put)
        {
            size_t len = tmp_begin_pos.size();
            if (len)
            {
                for (size_t j=0; j < len; ++j)
                {
                    result->emplace_back(TripleIntNode(tmp_begin_pos[j], tmp_begin_pos[j] + tmp_match[j]->level, tmp_match[j]->pos));
                }
                tmp_begin_pos.clear();
                tmp_match.clear();
            }
        }

        if (now_node == root)
        {
            // 若是最终跳到了root节点，或者一刚开始就处于root节点
            // 那么再搜寻一次root节点的子节点（词开头）
            // 因为刚才的循环在最后fail指向root并且未扫描root的子节点时就退出了
            auto iter = now_node->childs->find(word32[i]);
            if (iter != now_node->childs->end())
            {
                // 成功匹配上了词开头节点
                now_node = iter->second;
            }
            else
            {
                // 若是在root都匹配失败，就可直接去匹配下一个字符了
                continue;
            }
        }
        // 回溯失败节点，找到第一个词结尾节点
        last_match = now_node;
        while (last_match->pos < 0 && last_match != root)
        {
            last_match = last_match->fail;
        }
        if (last_match != root)
        {
            begin_pos = i - last_match->level;
            if (!tmp_begin_pos.empty())
            {
                // 因为实际上 tmp_begin_pos 是升序的
                // 所以可以通过判断开头来减少操作次数
                if (tmp_begin_pos.front() >= begin_pos)
                {
                    tmp_begin_pos.clear();
                    tmp_match.clear();
                }
                else
                {
                    // 将被覆盖结果清除
                    while (!tmp_begin_pos.empty() && tmp_begin_pos.back() >= begin_pos)
                    {
                        tmp_begin_pos.pop_back();
                        tmp_match.pop_back();
                    }
                }
            }
            tmp_begin_pos.push_back(begin_pos);
            tmp_match.push_back(last_match);
        }
    }
    // 检查是否还有剩余可能结果未添加进最终结果里
    if (!tmp_begin_pos.empty())
    {
        size_t len = tmp_begin_pos.size();
        for (size_t i=0; i < len; ++i)
        {
            result->emplace_back(TripleIntNode(tmp_begin_pos[i], tmp_begin_pos[i] + tmp_match[i]->level, tmp_match[i]->pos));
        }
    }
}

#endif // _ACDETECTOR_H_
