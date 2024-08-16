import Token, { ETokenType } from "./token"

export default class TokenList {
    tokens: Token[];

    constructor(tokens?: Token[]){
        this.tokens = tokens ?? [];
    }

    expectFront(...expected: ETokenType[]): TokenList {
        if(!expected.includes(this.front().type)){
            throw new Error(`Unexpected Token`);
        }
        return this;
    }

    expectBack(...expected: ETokenType[]): TokenList {
        if(!expected.includes(this.back().type)){
            throw new Error(`Unexpected Token`);
        }
        return this;
    }

    insertFront(token: Token): TokenList {
        this.tokens.unshift(token);
        return this;
    }

    insertBack(token: Token): TokenList {
        this.tokens.push(token);
        return this;
    }

    removeFront(): Token {
        return this.tokens.shift();
    }

    removeBack(): Token {
        return this.tokens.pop();
    }

    front(): Token {
        if(this.empty()){
            throw new Error("Expected Input");
        }
        return this.tokens[0];
    }

    back(): Token {
        if(this.empty()){
            throw new Error("Expected Input");
        }
        return this.tokens[this.tokens.length - 1];
    }

    notEmpty() {
        return this.tokens.length !== 0;
    }

    empty() {
        return this.tokens.length === 0;
    }
}