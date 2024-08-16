import { TokenDebugInfo } from "./token";

export class Symbol {
    debug: TokenDebugInfo;
    
}

export class Span {
    data: string;
    debug: TokenDebugInfo;
    constructor(data: string,debug: TokenDebugInfo){
        this.data = data;
        this.debug = debug;
    }
}



export class RenameSpan extends Span {
    
}

export class StructSymbol extends Symbol {

}