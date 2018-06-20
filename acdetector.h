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
            continue;
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

    int last_i = 0;
    TrieTreeNode *last_match = nullptr;
    bool is_put;

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
                is_put = true;
            }
            else
            {
                // 找到了匹配的子节点，移动到该子节点
                now_node = iter->second;
                break;
            }
        }
        if (last_match != nullptr && is_put)
        {
            result->emplace_back(TripleIntNode(last_i - last_match->level, last_i, last_match->pos));
            last_i = 0;
            last_match = nullptr;
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

        if (now_node->pos >= 0)
        {
            // 只取最长的
            last_i = i;
            last_match = now_node;
        }
    }
    // 检测最后一个字符是否为模式串结尾
    if (last_match != nullptr)
    {
        result->emplace_back(TripleIntNode(last_i - last_match->level, last_i, last_match->pos));
    }
}

#endif // _ACDETECTOR_H_
